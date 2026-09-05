# LP-OSM — Visor de mapes OpenStreetMap (Part 2)

Pràctica de **Programació 3**. Aplicació d'escriptori que
carrega fitxers OpenStreetMap (`.osm.xml`), en construeix el graf i el mostra. Permet 
calcular el camí més curt entre dos punts d'interès.

L'aplicació és **WPF (C#)** i utilitza una **DLL nativa (C++)** que fa el gruix del
processament del mapa. Només funciona a **Windows**.

## Estructura

| Carpeta | Contingut |
|---|---|
| `LP-OSM-VS2022.sln` | Solució de Visual Studio 2022 (2 projectes) |
| `LP-OSM-WPF-2022/` | Front-end WPF en C# (`netcoreapp3.1`, `x64`) |
| `LP-OSM-DLL-2022/` | DLL nativa en C++ (toolset v143), fa servir Xerces-C per llegir l'XML |
| `third-party/` | Xerces-C 3.2 (headers, `.lib` i `.dll`) — inclòs perquè compili directament |
| `mapesOSM/` | Mapes d'exemple (`UAB`, `Cerdanyola`, `Eiffel`, `Petit`) |

## Requisits

- **Windows 10/11 x64**
- **Visual Studio 2022** amb les càrregues de treball:
  - *Desarrollo de escritorio con C++* (toolset v143 + Windows SDK 10)
  - *Desarrollo de escritorio de .NET*
- **.NET Core 3.1 SDK** — <https://dotnet.microsoft.com/download/dotnet/3.1>

> No es pot compilar amb VS Code ni amb `dotnet build`: el projecte `LP-OSM-DLL-2022.vcxproj`
> és C++ natiu i necessita MSBuild + el compilador MSVC de Visual Studio.

## Com compilar i executar

1. Obrir `LP-OSM-VS2022.sln` amb Visual Studio 2022.
2. Barra superior: configuració **Debug**, plataforma **x64**.
3. Projecte d'inici: **LP-OSM-WPF-2022**.
4. `F5`.
5. Dins l'aplicació, obrir un dels mapes de `mapesOSM/`.

## Notes

- Els binaris de Xerces-C (`third-party/`) es distribueixen amb el repositori per comoditat;
  són programari de tercers (Apache License 2.0).
- Les carpetes de compilació (`bin/`, `obj/`, `x64/`, `.vs/`) estan excloses via `.gitignore`.
