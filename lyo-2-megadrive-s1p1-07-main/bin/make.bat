SolutionGenerator.exe -bp -make

:: --- CONFIGURATION ---
set "FICHIER=%~dp0..\ide\Game\Game.vcxproj"

set "BALISE_CIBLE=<PlatformToolset>v143</PlatformToolset>"
set "NOUVELLE_VALEUR=<PlatformToolset>v145</PlatformToolset>"
:: ---------------------

:: Vérification si le fichier existe bien à l'emplacement relatif
if not exist "%FICHIER%" (exit /b)

:: Appel de PowerShell avec gestion des apostrophes pour les chemins complexes
powershell -Command "$chemin = Get-Item '%FICHIER%'; (Get-Content $chemin.FullName -Raw) -replace '%BALISE_CIBLE%', '%NOUVELLE_VALEUR%' | Set-Content $chemin.FullName"