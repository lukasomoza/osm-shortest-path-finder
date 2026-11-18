using Microsoft.Web.WebView2.Core;
using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text.Json;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;
using LP_OSM_2223_WPF;

namespace LS_OSM_2223_WPF
{
    /// <summary>
    /// WPF front-end. All map processing (XML parsing, graph construction,
    /// nearest-node search and Dijkstra) happens in the native C++ DLL.
    /// The map itself is rendered with Leaflet inside a WebView2 control.
    /// </summary>
    public partial class MainWindow : Window
    {
        // ---------- Interop with the native C++ DLL ----------
        [StructLayout(LayoutKind.Sequential)]
        public struct PoiBridge
        {
            public int i;
            public double lat;
            public double lon;
            public int color;
            public IntPtr title;
        }

        public struct Way
        {
            public double[] lat;
            public double[] lon;
        }

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate IntPtr AllocateResultOfStrategyArray(IntPtr length);

        [DllImport("OsmEngine.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void loadOSMData(char[] map);

        [DllImport("OsmEngine.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void renderPois(AllocateResultOfStrategyArray allocator);

        [DllImport("OsmEngine.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool renderWays(int offset, AllocateResultOfStrategyArray allocatorLats, AllocateResultOfStrategyArray allocatorLons);

        [DllImport("OsmEngine.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void shortestPath(int idxFrom, int idxTo, AllocateResultOfStrategyArray allocatorLats, AllocateResultOfStrategyArray allocatorLons);

        // ---------- State ----------
        private BackgroundWorker loadWorker;
        private BackgroundWorker pathWorker;
        private FileSystemWatcher watcher;

        private PoiBridge[] poiBridges = new PoiBridge[0];
        private string[] poiTitles = new string[0];
        private readonly List<Way> ways = new List<Way>();
        private Way? lastPath;

        private int idxFrom;
        private int idxTo;
        private char[] loadedMap;
        private string loadedMapName;
        private readonly Stopwatch timer = new Stopwatch();

        private bool mapReady;
        private bool showRoads = true;
        private bool showPois = true;

        public MainWindow()
        {
            InitializeComponent();
            InitializeLog();
            InitializeWorkers();
            Loaded += async (s, e) => await InitializeMapAsync();
        }

        // ---------- Map (WebView2 + Leaflet) ----------
        private async Task InitializeMapAsync()
        {
            try
            {
                string userData = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "OsmPathFinder");
                var env = await CoreWebView2Environment.CreateAsync(null, userData);
                await mapView.EnsureCoreWebView2Async(env);

                string webDir = Path.Combine(AppContext.BaseDirectory, "web");
                mapView.CoreWebView2.SetVirtualHostNameToFolderMapping("app.local", webDir, CoreWebView2HostResourceAccessKind.Allow);
                mapView.CoreWebView2.Settings.AreDefaultContextMenusEnabled = false;
                mapView.CoreWebView2.Settings.AreDevToolsEnabled = Debugger.IsAttached;
                mapView.NavigationCompleted += (s, e) => mapReady = e.IsSuccess;
                mapView.Source = new Uri("https://app.local/map.html");
            }
            catch (Exception ex)
            {
                Log("Could not start the map view (is the WebView2 runtime installed?): " + ex.Message);
            }
        }

        private async Task Js(string script)
        {
            if (!mapReady) return;
            await mapView.CoreWebView2.ExecuteScriptAsync(script);
        }

        private static string Json(object value) => JsonSerializer.Serialize(value);

        private static double[][] ToLatLngs(Way w) =>
            Enumerable.Range(0, w.lat.Length).Select(i => new[] { w.lat[i], w.lon[i] }).ToArray();

        // ---------- Status panel ----------
        private void InitializeLog()
        {
            // The DLL can write messages to Log.txt; show them in the status panel
            watcher = new FileSystemWatcher
            {
                Path = AppContext.BaseDirectory,
                Filter = "Log.txt",
                NotifyFilter = NotifyFilters.LastWrite
            };
            watcher.Changed += Watcher_Changed;
            watcher.EnableRaisingEvents = true;
        }

        private void Log(string message, bool reset = false)
        {
            string line = $"[{DateTime.Now:HH:mm:ss}] {message}";
            logText.Text = reset ? line : logText.Text + Environment.NewLine + line;
            logScroll.ScrollToEnd();
        }

        private void Watcher_Changed(object sender, FileSystemEventArgs e)
        {
            watcher.EnableRaisingEvents = false;
            Dispatcher.BeginInvoke(new Action(() =>
            {
                try
                {
                    using var fileStream = new FileStream(e.FullPath, FileMode.Open, FileAccess.Read, FileShare.ReadWrite);
                    using var reader = new StreamReader(fileStream);
                    Log("DLL: " + reader.ReadToEnd().Trim());
                }
                catch (IOException) { }
                watcher.EnableRaisingEvents = true;
            }));
        }

        private void SetBusy(bool busy)
        {
            Cursor = busy ? Cursors.Wait : Cursors.Arrow;
            btnOpen.IsEnabled = !busy;
            bool loaded = poiBridges.Length > 0 || ways.Count > 0;
            btnRoute.IsEnabled = !busy && poiBridges.Length > 1;
            btnWays.IsEnabled = !busy && loaded;
            btnPois.IsEnabled = !busy && loaded;
        }

        private void InitializeWorkers()
        {
            loadWorker = new BackgroundWorker();
            loadWorker.DoWork += LoadWorker_DoWork;
            loadWorker.RunWorkerCompleted += LoadWorker_RunWorkerCompleted;

            pathWorker = new BackgroundWorker();
            pathWorker.DoWork += PathWorker_DoWork;
            pathWorker.RunWorkerCompleted += PathWorker_RunWorkerCompleted;
        }

        // ---------- Map loading ----------
        private async void MenuItem_Click(object sender, RoutedEventArgs e)
        {
            var dialog = new OpenFileDialog
            {
                Title = "Open OpenStreetMap file",
                Filter = "OpenStreetMap XML (*.osm.xml;*.osm;*.xml)|*.osm.xml;*.osm;*.xml|All files (*.*)|*.*"
            };
            string mapsDir = FindMapsFolder();
            if (mapsDir != null) dialog.InitialDirectory = mapsDir;

            if (dialog.ShowDialog() != true)
                return;

            poiBridges = new PoiBridge[0];
            poiTitles = new string[0];
            ways.Clear();
            lastPath = null;
            cmbFromPois.Items.Clear();
            cmbToPois.Items.Clear();
            await Js("osm.reset()");

            loadedMapName = Path.GetFileName(dialog.FileName);
            loadedMap = (dialog.FileName + "\0").ToCharArray();

            Log($"Loading {loadedMapName}…", reset: true);
            SetBusy(true);
            timer.Restart();
            loadWorker.RunWorkerAsync();
        }

        private static string FindMapsFolder()
        {
            var dir = new DirectoryInfo(AppContext.BaseDirectory);
            while (dir != null)
            {
                string candidate = Path.Combine(dir.FullName, "maps");
                if (Directory.Exists(candidate)) return candidate;
                dir = dir.Parent;
            }
            return null;
        }

        private void LoadWorker_DoWork(object sender, DoWorkEventArgs e)
        {
            loadOSMData(loadedMap);

            using (var pa = new PinnedArray<PoiBridge>())
            {
                renderPois(pa.CreateArray);
                poiBridges = pa.Array ?? new PoiBridge[0];
                poiTitles = poiBridges.Select(p => Marshal.PtrToStringAnsi(p.title) ?? "").ToArray();
            }

            bool nextWay;
            int i = 0;
            do
            {
                using var pLat = new PinnedArray<double>();
                using var pLon = new PinnedArray<double>();
                nextWay = renderWays(i, pLat.CreateArray, pLon.CreateArray);
                if (pLat.Array != null && pLat.Array.Length > 0)
                    ways.Add(new Way { lat = pLat.Array, lon = pLon.Array });
                i++;
            } while (nextWay);
        }

        private async void LoadWorker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            timer.Stop();
            if (e.Error != null)
            {
                Log("ERROR: " + e.Error.Message);
                SetBusy(false);
                return;
            }

            foreach (string title in poiTitles)
            {
                cmbFromPois.Items.Add(title);
                cmbToPois.Items.Add(title);
            }
            if (cmbFromPois.Items.Count > 1)
            {
                cmbFromPois.SelectedIndex = 0;
                cmbToPois.SelectedIndex = 1;
            }

            Log($"Loaded in {timer.ElapsedMilliseconds:N0} ms: {ways.Count} roads, {poiBridges.Length} points of interest.");

            var pois = poiBridges.Select((p, i) => new { lat = p.lat, lon = p.lon, color = p.color, title = poiTitles[i] });
            await Js($"osm.setRoads({Json(ways.Select(ToLatLngs))})");
            await Js($"osm.setPois({Json(pois)})");
            showRoads = showPois = true;
            await Js("osm.show('roads', true); osm.show('pois', true); osm.fit()");
            SetBusy(false);
        }

        // ---------- Layers ----------
        private async void VisualitzaCamins_Click(object sender, RoutedEventArgs e)
        {
            showRoads = !showRoads;
            await Js($"osm.show('roads', {showRoads.ToString().ToLower()})");
        }

        private async void VisualitzaPdis_Click(object sender, RoutedEventArgs e)
        {
            showPois = !showPois;
            await Js($"osm.show('pois', {showPois.ToString().ToLower()})");
        }

        private async void Clear_Click(object sender, RoutedEventArgs e)
        {
            showRoads = showPois = false;
            await Js("osm.show('roads', false); osm.show('pois', false); osm.setRoute([])");
        }

        // ---------- Shortest path ----------
        private void ShortestPath_Click(object sender, RoutedEventArgs e)
        {
            idxFrom = cmbFromPois.SelectedIndex;
            idxTo = cmbToPois.SelectedIndex;
            if (idxFrom < 0 || idxTo < 0)
                return;
            if (idxFrom == idxTo)
            {
                Log("Origin and destination are the same point.");
                return;
            }

            Log($"Routing: {poiTitles[idxFrom]} → {poiTitles[idxTo]}…");
            SetBusy(true);
            timer.Restart();
            pathWorker.RunWorkerAsync();
        }

        private void PathWorker_DoWork(object sender, DoWorkEventArgs e)
        {
            lastPath = null;
            using var pLat = new PinnedArray<double>();
            using var pLon = new PinnedArray<double>();
            shortestPath(idxFrom, idxTo, pLat.CreateArray, pLon.CreateArray);
            if (pLat.Array != null && pLat.Array.Length > 1)
                lastPath = new Way { lat = pLat.Array, lon = pLon.Array };
        }

        private async void PathWorker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            timer.Stop();
            SetBusy(false);

            if (e.Error != null)
            {
                Log("ERROR: " + e.Error.Message);
                return;
            }
            if (lastPath == null)
            {
                Log("No route found between these two points.");
                await Js("osm.setRoute([])");
                return;
            }

            Way path = lastPath.Value;
            double km = 0;
            for (int i = 1; i < path.lat.Length; i++)
                km += Haversine(path.lat[i - 1], path.lon[i - 1], path.lat[i], path.lon[i]);

            await Js($"osm.setRoute({Json(ToLatLngs(path))}, {Json(poiTitles[idxFrom])}, {Json(poiTitles[idxTo])})");
            Log($"Route found in {timer.ElapsedMilliseconds:N0} ms: {km * 1000:N0} m, {path.lat.Length} nodes.");
        }

        private static double Haversine(double lat1, double lon1, double lat2, double lon2)
        {
            const double R = 6371.0;
            double dLat = (lat2 - lat1) * Math.PI / 180, dLon = (lon2 - lon1) * Math.PI / 180;
            double a = Math.Sin(dLat / 2) * Math.Sin(dLat / 2) +
                       Math.Cos(lat1 * Math.PI / 180) * Math.Cos(lat2 * Math.PI / 180) * Math.Sin(dLon / 2) * Math.Sin(dLon / 2);
            return 2 * R * Math.Asin(Math.Sqrt(a));
        }
    }
}
