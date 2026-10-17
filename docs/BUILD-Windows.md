# Сборка GPXSee и релизного инсталлятора под Windows

Основано на официальном CI-процессе репозитория
(`.github/workflows/windows.yml`) и `README.md`. Версия в репозитории — **16.18**.

## 1. Что нужно установить (один раз)

1. **Visual Studio 2022** (Community или Build Tools) с компонентом
   **«Разработка классических приложений на C++»** (MSVC + Windows SDK).
   Официальные сборки GPXSee делаются компилятором MSVC.
2. **Qt 6.12.x** (open source), сборка под MSVC x64, с модулями:
   - базовый Qt (Core, Gui, Widgets, Network и т. д.);
   - **qtpositioning**, **qtserialport**, **qtimageformats**, **qtmultimedia**.

   Минимум по README — Qt6 ≥ 6.2 (или Qt5 ≥ 5.15), но официальные релизы
   собираются на **6.12.0** — бери эту ветку, чтобы совпадать с ними.

   Способы установки Qt:
   - **Qt Online Installer** с qt.io (нужен бесплатный аккаунт): выбрать
     Qt 6.12.x → MSVC 2022 64-bit + перечисленные модули.
   - Либо **aqtinstall** (без аккаунта):
     ```
     pip install aqtinstall
     aqt install-qt windows desktop 6.12.0 win64_msvc2022_64 -m qtpositioning qtserialport qtimageformats qtmultimedia
     ```
3. **NSIS 3.x** (нужен только для сборки инсталлятора) — https://nsis.sourceforge.io.
4. Zlib отдельно не нужен: на Windows используется zlib из поставки Qt.

## 2. Подготовка командной строки

Собирать нужно в окружении, где доступны **и** MSVC, **и** Qt:

1. Открыть **«x64 Native Tools Command Prompt for VS 2022»** (ярлык ставится
   вместе с Visual Studio) — он настраивает MSVC.
2. Добавить в `PATH` каталог `bin` своей сборки Qt, например:
   ```
   set PATH=C:\Qt\6.12.0\msvc2022_64\bin;%PATH%
   ```
   (путь зависит от того, куда установлен Qt).

Проверка: команды `qmake -v`, `nmake /?` и `windeployqt -?` должны работать.

## 3. Простая сборка (запустить программу локально)

Из корня репозитория:

```bat
lrelease gpxsee.pro        :: собрать переводы (.qm); можно lrelease-pro gpxsee.pro
qmake gpxsee.pro           :: сгенерировать Makefile
nmake release              :: собрать релизную сборку
```

Результат: `release\GPXSee.exe`.

Чтобы exe запускался вне среды Qt, положить рядом нужные библиотеки:
```bat
windeployqt --release release\GPXSee.exe
```
После этого `release\GPXSee.exe` запускается самостоятельно.

> Отладочная сборка: `nmake debug` → `debug\GPXSee.exe`.
> Пересобрать с нуля: `nmake distclean` (или удалить Makefile) → снова `qmake` → `nmake`.

## 4. Сборка релизного инсталлятора (.exe-установщик)

После шага 3 (есть `release\GPXSee.exe`), из корня репозитория:

```bat
md installer
copy release\GPXSee.exe installer

:: развернуть зависимости Qt рядом с exe (как в CI — без ненужных SQL-плагинов)
windeployqt --exclude-plugins qsqlibase,qsqlmimer,qsqloci,qsqlodbc,qsqlpsql,qstylekitstyle --release installer\GPXSee.exe

:: данные и ресурсы, которые кладёт официальный установщик
copy pkg\windows\gpxsee64.nsi installer
xcopy data\CRS   installer\CRS   /i
xcopy data\maps  installer\maps  /i
xcopy data\style installer\style /s /i
xcopy lang\*.qm  installer\translations\ /sy
xcopy icons\symbols installer\symbols /i
copy licence.txt installer

:: собрать инсталлятор NSIS (x64)
makensis /Dx64 installer\gpxsee64.nsi
```

Результат: `installer\GPXSee-16.18_x64.exe` — готовый установщик.

## 5. Примечания

- **ARM64:** то же самое, но командная строка — ARM64-вариант VS shell, а NSIS
  вызывается как `makensis /Darm64` (имя файла станет `GPXSee-16.18_arm64.exe`).
- **Цифровая подпись** (signtool) из CI — только при наличии сертификата; для
  личных сборок не нужна, шаг пропускается.
- **Версия** берётся из `gpxsee.pro` (`VERSION = 16.18`) и из
  `pkg\windows\gpxsee64.nsi` (`!define VERSION "16.18"`). При выпуске своей
  версии менять в обоих местах.
- **Qt5** (≥ 5.15) тоже соберётся, но рекомендуется Qt6 для совпадения с
  официальными релизами.
