# ClamAV-GUI 1.4.6 — Журнал изменений (сессии 2026-09-10, 2026-09-11)

## 1. Добавление иконки русского языка

Создана иконка `icons/ru_RU.png` (флаг России, 32×21 px, PNG RGB) в стиле остальных языковых иконок.

### Обновлённые файлы сборки:
- `resources.qrc` — добавлена строка `<file>icons/ru_RU.png</file>`
- `clamav-gui.pro`:
  - `TRANSLATIONS += translations/clamav-gui-ru_RU.ts` и `translations/clamav-ru_RU.ts`
  - `translation.files` — добавлены `.qm` файлы для ru_RU
  - `langicons.files` — добавлена `icons/ru_RU.png`

Язык обнаруживается динамически по `.qm` файлам, иконка подгружается как `languageicons/ru_RU.png`.

---

## 2. Исправление опечаток

| Опечатка | Исправление | Файлы |
|----------|-------------|-------|
| `occured` | `occurred` | `src/freshclamsetter.cpp`, `src/clamav_gui.cpp` |
| `programm` | `program` | `src/freshclamsetter.cpp` (×3) |
| `m_supressMessage` | `m_suppressMessage` | `src/setuptab.h`, `src/setuptab.cpp` (×3) |
| `flatpack-spawn` | `flatpak-spawn` | `src/freshclamsetter.cpp` |
| `isRunninginFlatPak()` | `isRunningInFlatpak()` | `src/toolbox.h/.cpp`, `src/setuptab.cpp`, `src/firstrunwindow.cpp`, `src/main.cpp`, `src/infodialog.cpp`, `src/freshclamsetter.cpp` |
| `isRunninginAppImage()` | `isRunningInAppImage()` | `src/toolbox.h/.cpp`, `src/setuptab.cpp`, `src/firstrunwindow.cpp` |
| `dragablePushButton` | `draggablePushButton` | `src/draggablepushbutton.h/.cpp`, `src/scantab.h/.cpp`, `src/profilewizarddialog.h`, `ui/draggablepushbutton.ui` |

### Переименованные файлы:
- `src/dragablepushbutton.h` → `src/draggablepushbutton.h`
- `src/dragablepushbutton.cpp` → `src/draggablepushbutton.cpp`
- `ui/dragablepushbutton.ui` → `ui/draggablepushbutton.ui`

---

## 3. Исправление уязвимостей безопасности

### 3.1. Command Injection (Root RCE) — УСТРАНЕНО

**Корневая причина:** паттерн «записать bash-скрипт в предсказуемый путь → выполнить через sudo/pkexec». Значения из `~/.clamav-gui/settings.ini` вставлялись в shell-строку без санитизации.

**Решение:** все bash-скрипты заменены на прямые вызовы `QProcess` с аргументами (без шелла). PID-значения валидируются функцией `isValidPid()`.

| ID | Файл | Было | Стало |
|----|------|------|-------|
| CI-1 | `freshclamsetter.cpp` | Запись `startfreshclam.sh` + sudo | `startProcess(m_updater, m_sudoGUI, {binary, "--show-progress", ...})` |
| CI-2 | `freshclamsetter.cpp` | Запись `stopfreshclam.sh` с `kill && rm` | `QProcess::execute(sudo, {"kill", "-sigterm", pid})` + `QFile::remove(pidfile)` |
| CI-3 | `clamdmanager.cpp` | Запись `startclamd.sh` с `clamd && clamonacc` | Два отдельных `startProcess()` вызова |
| CI-4 | `clamdmanager.cpp` | Запись `stopclamd.sh` с `kill -9 <pid>` | `QProcess::execute(sudo, {"kill", "-9", pid})` с валидацией PID |
| CI-5 | `clamdmanager.cpp` | Запись `restartclamd.sh` | Прямые QProcess вызовы |
| CI-6 | `clamdmanager.cpp` | Запись `startclamd.sh` (restart) | Kill через QProcess + start через QProcess |
| CI-7 | `toolbox.cpp` | `bash -c "pidof -s progname"` | `QProcess::start("pidof", {"-s", progname})` |

### 3.2. Insecure Permissions — ИСПРАВЛЕНО

| ID | Файл | Было | Стало | Описание |
|----|------|------|-------|----------|
| IP-1 | `freshclamsetter.cpp` | `0666` (world-writable) | `0644` | Лог-файлы `update.log`, `freshclam.log` |
| IP-2 | `firstrunwindow.cpp` | `0777` (world-writable+exec) | `0700` | Директория `~/.clamav-gui/signatures` |
| IP-3 | `toolbox.cpp` | `0770` (group-writable) | `0755` | Файл `.desktop` для меню Dolphin |

### 3.3. Path Traversal — ЗАЩИЩЕНО

Добавлена функция `isValidProfileName()` в `toolbox.h/.cpp`:
- Запрещает `/`, `..`, null-байты
- Разрешает только `[a-zA-Z0-9_-]`

Применена в:
- `src/profilewizarddialog.cpp` — `closeEvent()`, `slot_createButtonClicked()`
- `src/scheduler.cpp` — цикл перебора профилей, `startScanJob()`

### 3.4. Race Conditions / TOCTOU — УСТРАНЕНО

Устранено автоматически вместе с удалением bash-скриптов:
- **RC-1** (PID reuse race): PID теперь передаётся напрямую в `kill` без задержки на запись/запуск скрипта
- **RC-2** (stale state): устранена гонка на общий скрипт-файл
- **RC-3** (script swap TOCTOU): скрипты больше не пишутся на предсказуемые пути
- **RC-4** (PID file race): удаление PID-файла выполняется после kill, без шелла

### 3.5. Новые helper-функции в `toolbox.h/.cpp`

```cpp
bool isValidPid(const QString &pid);          // Проверяет, что строка содержит только цифры
bool isValidProfileName(const QString &name); // Проверяет имя профиля (без /, .., спецсимволов)
```

---

## 4. Синхронизация файлов переводов (.ts) с кодом

Обнаружено и исправлено несоответствие в 9 файлах `clamav-gui-*.ts` (кроме `ru_RU`):

| Устаревшая строка | Актуальная строка (в коде) | Вхождений на файл |
|---|---|---|
| `an error occured!` | `an error occurred!` | 1 |
| `Select a programm that ...` | `Select a program that ...` | 3 |
| `<name>dragablePushButton</name>` | `<name>draggablePushButton</name>` | 1 |
| `../ui/dragablepushbutton.ui` | `../ui/draggablepushbutton.ui` | 1 |

**Затронутые файлы:** `da_DK`, `de_DE`, `en_GB`, `es_ES`, `fr_FR`, `it_IT`, `pt_PT`, `uk_UA`, `zh_CN` — всего 54 замены.

В `clamav-gui-en_GB.ts` дополнительно исправлены дубликаты опечаток в тегах `<translation>`.

---

## 5. Пересборка файлов .qm

Все 20 файлов `.qm` пересозданы из актуальных `.ts` с помощью `lrelease` (Qt6):

```bash
/usr/lib/qt6/bin/lrelease translations/<file>.ts -qm translations/<file>.qm
```

Результат: все переводы завершены (0 unfinished), ошибок нет.

---

## 6. Исправление выбора языка в интерфейсе

### Проблема
При запуске из build-каталога (без `make install`) языковой ComboBox был пуст: код искал `.qm` файлы по пути `<app_dir>/../share/clamav-gui/`, который не существует в режиме разработки.

### Корневая причина
1. Отсутствовал fallback на каталог приложения при неработающем стандартном пути
2. В `install.cmake` не были указаны `clamav-gui-ru_RU.qm`, `clamav-ru_RU.qm` и иконка `ru_RU.png`
3. Иконки `languageicons/` не копировались в build-директорию при сборке через CMake

### Исправления

| Файл | Изменение |
|------|-----------|
| `src/main.cpp` | Fallback пути загрузки `.qm`: если `<app>/../share/clamav-gui/` не существует, используется каталог приложения |
| `src/setuptab.cpp` | Аналогичный fallback для обнаружения языков в ComboBox |
| `src/firstrunwindow.cpp` | Аналогичный fallback для окна первого запуска |
| `install.cmake` | Добавлены `clamav-gui-ru_RU.qm`, `clamav-ru_RU.qm` в список установки; добавлена `icons/ru_RU.png` в `languageicons/` |
| `CMakeLists.txt` | POST_BUILD: создание `languageicons/` и копирование всех иконок из `icons/` в build-каталог |

---

## 7. Перевод непереведённых элементов интерфейса на русский язык

### Обнаруженные проблемы

По скриншотам интерфейса (папка `сканы/`) выявлены строки, отображаемые на английском:

| Строка | Причина | Файл |
|--------|---------|------|
| `Selected Directories` | Отсутствует в `.ts` | `ui/scantab.ui:395` |
| `Scanner:` / `Database:` / `Date:` | Не обёрнуты в `tr()` | `src/setuptab.cpp:86-88`, `src/clamav_gui.cpp:177-179` |
| `Clamd Scan on Access` | Строка в `.ui` не совпадает с `.ts` (там `Clamd && Scan on Access`) | `ui/clamdmanager.ui:37` |
| `Clamd Scan on Access Settings` | Аналогично | `ui/clamdmanager.ui:133` |

### Исправления в коде

**`src/setuptab.cpp` (строки 86–88):**
```cpp
// Было:
systemInfo = "...<b>Scanner: <font...>" + scannerVersion + "...Database: ...";
systemInfo += "Date: ...";
// Стало:
systemInfo = "...<b>" + tr("Scanner:") + " <font...>" + scannerVersion + "...";
systemInfo += tr("Date:") + " ...";
```

**`src/clamav_gui.cpp` (строки 177–179):** — аналогичная замена.

### Добавленные переводы (`translations/clamav-gui-ru_RU.ts`)

| Source | Translation | Context |
|--------|-------------|---------|
| `Selected Directories` | Выбранные директории | scanTab |
| `Clamd Scan on Access` | Clamd и сканирование при доступе | clamdManager |
| `Clamd Scan on Access Settings` | Настройки Clamd и сканирования при доступе | clamdManager |
| `Scanner:` | Сканер: | setupTab, clamav_gui |
| `Database:` | База данных: | setupTab, clamav_gui |
| `Date:` | Дата: | setupTab, clamav_gui |

Файл `.qm` пересоздан: **559 переводов, 0 незавершённых**.

---

## 8. Русская man-страница

Создана папка `man/ru/` с переведённой man-страницей `clamav-gui.1.gz`.

### Содержимое перевода:
- **НАЗВАНИЕ** — ClamAV-GUI — графический интерфейс для ClamAV
- **СИНТАКСИС** — `clamav-gui [--force] [--scan] [--language] [--setlang xx_XX]`
- **ОПИСАНИЕ** — назначение программы
- **ПАРАМЕТРЫ** — `--force`, `--scan`, `--language`, `--setlang`
- **АВТОР** — Joerg Zopes

### Обновлённый файл сборки:
- `clamav-gui.pro` — добавлены `manpages_ru` в `INSTALLS`, путь `/usr/share/man/ru/man1`, файл `man/ru/clamav-gui.1.gz`

Установка через CMake (`install.cmake`) не требует изменений — используется `install(DIRECTORY man ...)`.

---

## 9. Увеличение ширины кнопок в Менеджере профилей

### Проблема
На вкладке «Менеджер профилей» текст русских надписей кнопок обрезался: «Изменить профил…», «Добавить профил…», «Удалить профил…». Причина — фиксированная ширина 140 px, недостаточная для русского текста с иконкой 24 px.

### Исправление
Файл `ui/profilemanager.ui` — ширина (minimumSize + maximumSize) увеличена с **140 → 180 px** для трёх кнопок:

| Кнопка | Имя виджета | Английский текст | Русский перевод |
|--------|-------------|-----------------|----------------|
| Удалить | `pushButton` | erase Profile | Удалить профиль |
| Изменить | `editProfileButton` | edit Profile | Изменить профиль |
| Добавить | `addProfileButton` | add Profile | Добавить профиль |

---

## 10. Перевод «Direct Scan» на русский язык

### Проблема
В мастере профилей и просмотрщике журналов текст «Direct Scan» отображался на английском — строки не были обёрнуты в `tr()`.

### Исправления в коде

| Файл | Было | Стало |
|------|------|-------|
| `src/schedulescanobject.cpp:17` | `setText("Direct Scan")` | `setText(tr("Direct Scan"))` |
| `src/logviewerobject.cpp:43` | `addItem("Direct Scan")` | `addItem(tr("Direct Scan"))` |
| `src/logviewerobject.cpp:75` | `if (profile == "Direct Scan")` | `if (profile == tr("Direct Scan"))` |
| `src/main.cpp:144` | `setWindowTitle("Direct Scan-Job")` | `setWindowTitle(QCoreApplication::translate("main", "Direct Scan-Job"))` |

Внутренний идентификатор `"Direct Scan"` (передаваемый в конструктор `scheduleScanObject`) оставлен на английском для корректной работы логики.

### Добавленные переводы (`translations/clamav-gui-ru_RU.ts`)

| Source | Translation | Context |
|--------|-------------|---------|
| `Direct Scan` | Прямое сканирование | logViewerObject, scheduleScanObject |
| `Direct Scan-Job` | Прямое сканирование | main |

---

## 11. Исправление обрезки текста «Имя профиля»

### Проблема
В мастере профилей метка «Имя профиля:» обрезалась («Имя профил…») из-за фиксированной ширины 100 px, недостаточной для русского текста при шрифте 13 pt (фактическая ширина ~122–130 px).

### Исправление
Файл `ui/profilewizarddialog.ui` — `maximumSize.width` метки `profileNameLabel` увеличена с **100 → 150 px**. Поле ввода `profileNameLineEdit` автоматически уменьшается (оба виджета в одной сетке `QGridLayout`).

---

## 12. Исправление перевода приветственного текста Мастера профилей

### Проблема
HTML-блок приветствия на первой странице Мастера профилей («Welcome to the Profile Wizard...») отображался на английском, несмотря на наличие перевода в `.ts` файле.

### Корневая причина
В `<source>` строке `translations/clamav-gui-ru_RU.ts` было `font-family:'Noto Sans'`, а в актуальном `ui/profilewizarddialog.ui` — `font-family:'Sans Serif'`. Qt использует **точное совпадение** строки для поиска перевода, поэтому из-за этого одного расхождения весь HTML-блок не переводился.

### Исправление
Файл `translations/clamav-gui-ru_RU.ts` — в сообщении для `profilewizarddialog.ui:181` заменено `'Noto Sans'` → `'Sans Serif'` в обеих частях (`<source>` и `<translation>`). Файл `.qm` пересобран.

---

## 13. Исправление обрезки текста «Папка под наблюдением»

### Проблема
На вкладке Clamd метка «Folder under monitoring» (русский перевод: «Папка под наблюдением») обрезалась из-за фиксированной ширины 169 px, недостаточной для русского жирного текста.

### Исправление
Файл `ui/clamdmanager.ui` — `maximumSize.width` метки `monitoringLabel` увеличена с **169 → 200 px**.

---

## 14. Исправление непереведённых описаний параметров ClamAV

### Проблема
На вкладке Clamd многие описания параметров (SHUTDOWN, RELOAD, VERSION, STATS, STREAM, DatabaseDirectory, CVDRepository, FIPS и др.) отображались на английском, несмотря на наличие переводов в `clamav-ru_RU.ts`.

### Корневая причина
Во всех файлах `translations/clamav-*.ts` (все 10 языков) в строках `<source>` содержался символ **U+2010 HYPHEN** (`‐`) внутри слов — артефакт переноса строк из man-страницы, захваченный при генерации `.ts` через `lupdate`. Пример: `UN‐ AVAILABLE` вместо `UNAVAILABLE`, `direc‐ tory` вместо `directory`.

Qt использует **точное совпадение** строки для поиска перевода. В рантайме текст из `man clamd.conf` приходит без U+2010, поэтому перевод не находиллся.

Дополнительно: путь `/etc/certs` в `.ts` не соответствовал фактическому `/etc/clamav/certs` в установленной версии ClamAV.

### Исправление

| Файл | Изменение |
|------|-----------|
| `translations/clamav-ru_RU.ts` | Удалены все 39 вхождений `U+2010 + пробел`; `/etc/certs` → `/etc/clamav/certs` |
| `translations/clamav-{da_DK,de_DE,en_GB,es_ES,fr_FR,it_IT,pt_PT,uk_UA,zh_CN}.ts` | Удалены 55–56 вхождений `U+2010 + пробел` (каждый файл); `/etc/certs` → `/etc/clamav/certs` |
| `translations/*.qm` | Все 20 файлов пересобраны через `lrelease` |

Результат: все `clamav-*.qm` — **0 незавершённых**, переводы работают для всех языков.

---

## 15. Дополнительные непереведённые строки интерфейса

При запуске программы обнаружены ещё строки, отображаемые на английском. Все исправлены добавлением `tr()` или созданием записей в `.ts`.

### Исправления в коде

| Файл | Строка | Было | Стало |
|------|--------|------|-------|
| `src/scantab.cpp:196` | — | `"Scanning : "` | `tr("Scanning : ")` |
| `src/optionsdialog.cpp:264-288` | ×3 | WARNING сообщения без tr() | Обёрнуты в `tr()` |
| `src/optionsdialog.cpp` | — | `"ClamAV Version:"` | `tr("ClamAV Version : ")` |
| `src/firstrunwindow.cpp:13` | — | `"ClamAV-GUI Basic Settings"` | `tr(...)` |
| `src/firstrunwindow.cpp:230` | ×2 | `"Database Owner : "` | `tr(...)` |
| `src/firstrunwindow.cpp:241` | ×2 | `"Application Group : "` (1 пробел) | `tr("Application Group  : ")` (2 пробела, как в .ui) |
| `src/partiallogobject.cpp:87` | — | `"INFO"` | `tr("INFO")` |
| `src/scheduler.cpp:172,175,178` | — | `"Never"` | `tr("Never")` |
| `src/scheduler.cpp:480` | ×2 | `"dd.MM.yyyy 'at' hh:mm"` | `tr(...)` |
| `src/scheduler.cpp` | — | `"Log-File : "` | `tr("Log-File : ")` |
| `src/setuptab.cpp:~94-164` | ×4 | Статусы без tr() | Явные if/else с `tr()` для каждого статуса |
| `src/setuptab.cpp:233` | — | `"Update available: "` | `tr(...)` |

### Исправление логической ошибки

| Файл | Было | Стало | Описание |
|------|------|-------|----------|
| `src/clamdmanager.cpp:680` | `"not Running"` | `"not running"` | Сравнение в setuptab.cpp было case-sensitive; заглавная R ломала логику |

### Добавленные переводы (`translations/clamav-gui-ru_RU.ts`)

| Source | Translation | Context |
|--------|-------------|---------|
| `Scanning : ` | Сканирование: | scanTab |
| `WARNING` | ПРЕДУПРЕЖДЕНИЕ | optionsDialog |
| `ClamAV Version : ` | Версия ClamAV: | optionsDialog |
| `ClamAV-GUI Basic Settings` | Базовые настройки ClamAV-GUI | firstRunWindow |
| `Database Owner : ` | Владелец базы данных: | firstRunWindow |
| `INFO` | ИНФО | partialLogObject |
| `Never` | Никогда | scheduler |
| `dd.MM.yyyy 'at' hh:mm` | dd.MM.yyyy в hh:mm | scheduler |
| `Log-File : ` | Файл журнала: | scheduler |
| `is running` | запущен | setupTab |
| `is down` | остановлен | setupTab |
| `starting up ...` | запускается... | setupTab |
| `shutting down ...` | останавливается... | setupTab |
| `Update available: ` | Доступно обновление: | setupTab |

---

## 16. Нормализация U+2010 в рантайме (clamdmanager.cpp)

### Проблема
В man page (`man clamd.conf`) длинные слова переносятся через **U+2010 HYPHEN** на новую строку (например, `pre‐\nvent`). Парсер приложения объединяет строки пробелом, в результате label содержит `pre‐ vent`. Qt ищет точное совпадение с `<source>` в `.ts`, где слово записано целиком (`prevent`) → перевод не находится.

Затронуто 4 параметра в man page:
| Перенос в man page | Слово после нормализации |
|---|---|
| `file‐\nname` | `filename` |
| `exe‐\ncutable` | `executable` |
| `exam‐\nple` | `example` |
| `pre‐\nvent` | `prevent` |

### Исправление
Добавлена нормализация label **до** передачи в конструктор опции (`src/clamdmanager.cpp`, после извлечения из parsed element):

```cpp
values.size() > 1?label = values.at(1):label="";
label.replace(QString(QChar(0x2010)) + " ", "");  // ← добавлено
```

Теперь все 4 параметра корректно сопоставляются с чистыми `<source>` строками в `clamav-ru_RU.ts`.

---

## 17. Исправление обрезанного source (CacheSize) и удаление дубликата

### Проблема 1: CacheSize
В `translations/clamav-ru_RU.ts` строка `<source>` для параметра `CacheSize` была **обрезана**:
```
...will be rounde...
```
Полный текст из man page:
```
...will be rounded up to the nearest square number. Default: 65536
```
Qt не находил совпадение → параметр отображался на английском.

### Исправление
`<source>` дополнен до полного текста. Перевод (уже существовал) сохранился.

### Проблема 2: Дубликат
Запись «Exclude a specific PUA category...» встречалась дважды (строки 450 и 942). `lrelease` выдавал предупреждение о дубликате.

### Исправление
Удалена повторная запись (строка 942). После пересборки: **236 переводов, 0 незавершённых, без предупреждений**.

---

## 18. Исправление пути бинарника в AppImage

### Проблема
AppImage запускался, но интерфейс был на английском, выбор языков недоступен. Причина — несоответствие путей установки:

| Компонент | Путь в AppImage | 
|-----------|-----------------|
| Бинарник | `bin/clamav-gui` (через `${CMAKE_INSTALL_BINDIR}`) |
| Переводы `.qm` | `usr/share/clamav-gui/` (хардкод в `install.cmake`) |
| Иконки языков | `usr/share/clamav-gui/languageicons/` |

Код ищет переводы по пути `<applicationDirPath>/../share/clamav-gui/`:
- Из `bin/` → `share/clamav-gui/` ← **не существует**
- Из `usr/bin/` → `usr/share/clamav-gui/` ← **существует** ✓

### Исправление
Файл `install.cmake`:
```cmake
# Было:
install(TARGETS clamav-gui RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Стало:
install(TARGETS clamav-gui RUNTIME DESTINATION usr/bin)
```

Теперь все компоненты AppImage находятся в едином FHS-дереве (`usr/bin`, `usr/share`).

---

## 19. Удаление дубликатов из install.cmake

### Проблема
Файл `install.cmake` содержал дублирующие правила установки, из-за чего в DEB/RPM/AppImage пакетах одни и те же файлы (или их варианты) устанавливались в **два** разных каталога:

| Файл | Дубликат (удалён) | Основной (оставлен) |
|------|-------------------|---------------------|
| `.desktop` | `/share/applications/` | `/usr/share/applications/` |
| icon64 `io.github...png` | `/share/icons/hicolor/64x64/apps/` | `/usr/share/icons/hicolor/64x64/apps/clamav-gui.png` |
| icon128 `io.github...png` | `/share/icons/hicolor/128x128/apps/` | `/usr/share/icons/hicolor/128x128/apps/clamav-gui.png` |
| icon256 `io.github...png` | `/share/icons/hicolor/256x256/apps/` | `/usr/share/icons/hicolor/256x256/apps/clamav-gui.png` |
| metainfo `.appdata.xml` | `/usr/share/metainfo/` | — |
| metainfo `.metainfo.xml` | `/share/metainfo/` | `/usr/share/metainfo/` |

### Исправление
Удалены все правила установки с нестандартными путями (`/share/...` без префикса `usr/`). Оставлены только корректные FHS-пути под `/usr/share/`. Для метайнфо использован современный формат `.metainfo.xml`. Иконка 256×256 переименована на стандартный `clamav-gui.png`.

Удалён также закомментированный мёртвый код в начале файла.

### Результат
Все три формата пакетов (DEB, RPM, AppImage) теперь содержат **только** файлы под `/usr/` без дубликатов.

---

## 20. Перевод «yes»/«no» в выпадающих списках вкладки Clamd

### Проблема
BOOL-параметры `clamd.conf` (например, `TestCommand`, `FixNesting`) отображали значения `yes` и `no` в `QComboBox` без перевода — строки хардкодились в парсере (`clamdmanager.cpp:1051`) и добавлялись в combobox как есть.

### Корневая причина
В `clamdconfcomboboxoption.cpp` элементы добавлялись через `addItem(value)` без `tr()`, а `getValue()` возвращал `currentText()` — т.е.displayed text = saved value.

### Исправление

**`src/clamdconfcomboboxoption.cpp`:**
- При добавлении элементов: если значение `"yes"` или `"no"`, отображается `tr("yes")` / `tr("no")`, а исходное значение хранится в `itemData`
- `getValue()` — возвращает `currentData().toString()` (исходное `"yes"`/`"no"` для записи в конфиг)
- `setValue()` — ищет по `findData()` с fallback на `findText()`
- Установка текущего значения — через `findData()` вместо `setCurrentText()`

**`translations/clamav-gui-ru_RU.ts`** (контекст `clamdconfcomboboxoption`):
| Source | Translation |
|--------|-------------|
| `yes` | да |
| `no` | нет |

Файл `.qm` пересобран: **577 переводов, 0 незавершённых**.

---

## 21. Исправление сборки AppImage (иконка в .desktop)

### Проблема
После удаления дубликатов из `install.cmake` сборка AppImage упала с ошибкой:
```
CPack Error: Could not find the Icon referenced in the desktop file: io.github.wusel1007.clamav-gui.png
```
Затем, после переименования иконки в `.desktop`:
```
CPack Error: CPACK_PACKAGE_ICON must match the file name referenced in the desktop file.
```

### Причина
В файле `extra/io.github.wusel1007.clamav-gui.desktop` была строка `Icon=io.github.wusel1007.clamav-gui`, но устанавливаемый файл иконки — `clamav-gui.png`. CPack требует точного совпадения имени иконки между `.desktop` и `CPACK_PACKAGE_ICON`.

### Исправление

| Файл | Изменение |
|------|-----------|
| `extra/io.github.wusel1007.clamav-gui.desktop` | `Icon=io.github.wusel1007.clamav-gui` → `Icon=clamav-gui` |
| `install.cmake` | `CPACK_PACKAGE_ICON` → `clamav-gui.png` |
| Корень проекта | Добавлена копия `clamav-gui.png` (256×256) для CPack |

### Результат
Все три формата пакетов (AppImage, DEB, RPM) собираются без ошибок.

---

## 22. Исправление записи «да»/«нет» вместо «yes»/«no» в конфигурационных файлах

### Проблема
При создании запланированного сканирования команда `clamscan` получала русские значения:
```
clamscan --leave-temps=да --recursive=да --allmatch=да --cross-fs=да ...
```
Аналогично в `freshclam.conf` записывались `LogSyslog нет`, `LogRotate да` и т.д.

### Корневая причина
Qt автоматически переводит элементы QComboBox из `.ui` файлов через систему переводов. Код использовал `currentText()` для чтения/записи значений в конфигурационные файлы, получая переведённый текст («да»/«нет») вместо оригинального («yes»/«no»).

### Исправление

Применён паттерн `itemData`: отображаемый текст переводится, а фактическое значение хранится в `QVariant` данных элемента.

| Файл | Что исправлено |
|------|---------------|
| `src/scanoptionyn.cpp` | Добавлен `setItemData(0,"yes")` / `setItemData(1,"no")`; все записи в профиль — через `currentData().toString()`; выбор по индексу вместо `setCurrentText()` |
| `src/freshclamsetter.cpp` | 4 combobox'а (LogSyslog, LogTime, LogRotate, LogVerbose): чтение через `findData()`+`setCurrentIndex()`, запись через `currentData().toString()` |
| `src/clamdconfcomboboxoption.cpp` | Слоты `slot_checkBoxClicked()` и `slot_comboBoxChanged()` — запись через `currentData().toString()` с fallback на `currentText()` |

### Исправленные данные пользователя

| Файл | Было | Стало |
|------|------|-------|
| `~/.clamav-gui/profiles/boot.ini` | `--leave-temps<equal>да`, `--recursive<equal>да`, `--allmatch<equal>да`, `--cross-fs<equal>да` | `...<equal>yes` (×4) |
| `~/.clamav-gui/freshclam.conf` | `LogSyslog нет`, `LogRotate да`, `LogTime да`, `LogVerbose да` | `... no/yes` (×4) |

### Результат
Команда сканирования формируется корректно: `--leave-temps=yes --recursive=yes --allmatch=yes --cross-fs=yes`. Все конфигурационные файлы содержат английские значения.

---

## 23. Исправление выбора языка во вкладке «Настройки» (AppImage)

### Проблема
При запуске из AppImage во вкладке «Настройки» ComboBox выбора языка был пуст — сменить язык после первого запуска было невозможно. В окне первого запуска (`firstrunwindow.cpp`) и при загрузке переводов (`main.cpp`) всё работало штатно.

### Корневая причина
В `src/setuptab.cpp::findTranslation()` путь к переводам для AppImage переопределялся на литеральный системный путь:
```cpp
if (isRunningInAppImage())
    translation_path = "/usr/share/clamav-gui/";
```
В AppImage файлы монтируются во временный каталог (`/tmp/.mount_XXXXX/usr/share/clamav-gui/`), а не в `/usr/share/clamav-gui/`. Путь не существовал → `QDir::entryList()` возвращал пустой список → ComboBox без элементов.

### Исправление
Удалена строка `if (isRunningInAppImage()) translation_path = "/usr/share/clamav-gui/";` из `setuptab.cpp`. Базовая логика уже корректно резолвит путь:
```cpp
translation_path = QCoreApplication::applicationDirPath() + "/../share/clamav-gui/";
// Для AppImage: <mount>/usr/bin/../share/clamav-gui/ = <mount>/usr/share/clamav-gui/ ✓
```

### Файл
- `src/setuptab.cpp` — удалено 2 строки

### Верификация всех форматов пакетов

| Формат | Бинарник | Переводы | `.qm` файлов | Статус |
|--------|----------|----------|:------------:|--------|
| AppImage | `<mount>/usr/bin/clamav-gui` | `<mount>/usr/share/clamav-gui/` | 20 | ✅ ComboBox заполнен |
| DEB | `/usr/bin/clamav-gui` | `/usr/share/clamav-gui/` | 20 | ✅ Путь корректен |
| RPM | `/usr/bin/clamav-gui` | `/usr/share/clamav-gui/` | 20 | ✅ Путь корректен |

Для всех трёх форматов `applicationDirPath()/../share/clamav-gui/` резолвится в существующий каталог с переводами. Баг был специфичен только для AppImage из-за литерального переопределения пути.

### Сборка AppImage (appimagetool, continuous build)

Эта версия `appimagetool` имеет особые требования к структуре AppDir:
1. Файл `.desktop` должен присутствовать **на корневом уровне** каталога (не только в `usr/share/applications/`)
2. Иконка (`<name>.png`, 256×256) — также на корневом уровне
3. Переменная окружения `ARCH=x86_64` обязательна

```bash
# Чистое дерево установки
rm -rf /tmp/appimage-root && mkdir -p /tmp/appimage-root
cmake --install . --prefix /tmp/appimage-root

# Корневые файлы, требуемые appimagetool
cp /tmp/appimage-root/usr/share/applications/io.github.wusel1007.clamav-gui.desktop /tmp/appimage-root/
cp /tmp/appimage-root/usr/share/icons/hicolor/256x256/apps/clamav-gui.png /tmp/appimage-root/

# Сборка
ARCH=x86_64 appimagetool /tmp/appimage-root ClamAV-GUI-1.4.6-x86_64.AppImage
```

---

## 24. Пакет для Arch Linux / AUR

Создана папка `arch/` со стандартными файлами для сборки пакета Arch Linux.

### Файлы

| Файл | Назначение |
|------|-----------|
| `arch/PKGBUILD` | Стандартный PKGBUILD для AUR (загрузка исходников из git) |
| `arch/build-local.sh` | Скрипт локальной сборки через `makepkg` без сети |

### Параметры пакета

| Поле | Значение |
|------|----------|
| pkgname | clamav-gui |
| pkgver | 1.4.6 |
| pkgrel | 1 |
| arch | x86_64, armv7h |
| license | GPL-3.0-or-later |
| depends | clamav, qt6-base |
| makedepends | cmake |

### Сборка

```bash
# Локальная сборка (без сети):
./arch/build-local.sh

# AUR-сборка (нужен интернет):
cd arch && makepkg -si
```

### Результат

- `arch/clamav-gui-1.4.6-1-x86_64.pkg.tar.zst` (1.7 MB)
- Валидирован через `pacman -Qip`
- Содержимое: бинарник, иконки (7 размеров), .desktop, 20 .qm файлов, languageicons, man-страницы (10 языков), metainfo, документация

---

## 25. Ревью и исправление критических дефектов (сессия 2026-09-11)

Полное ревью проекта выявило 15 проблем. 14 исправлены (одна — God class — отложена как архитектурная).

### 25.1. Dangling pointer UB в setupfilehandler.cpp

| Было | Стало |
|------|-------|
| `m_setupFileContent = stream.readAll().toLocal8Bit().constData()` | `m_setupFileContent = stream.readAll()` |

`toLocal8Bit()` создавал временный `QByteArray`, `.constData()` возвращал указатель на его буфер. После выхода из выражения буфер уничтожался → `m_setupFileContent` ссылался на освобождённую память. Каждое чтение настроек вызывало UB.

### 25.2. clamonacc опции передавались одним аргументом

**`clamdmanager.cpp`** (2 места):

| Было | Стало |
|------|-------|
| `accParams << clamonaccOptions` | `accParams << clamonaccOptions.split(" ", Qt::SkipEmptyParts)` |

Строка с опциями (например `-i /path -l log`) передавалась как **один** аргумент в `QProcess` вместо списка. On-access сканирование не работало.

### 25.3. findClamdProcess() — последний bash-shell вызов

| Было | Стало |
|------|-------|
| `runProg("bash", {"-c", "ps ax \| grep clamd"})` | `pidof("clamd")` + `runProg("ps", {"-o","args=","-p", clamdPid})` |

Устранён последний инвок shell'а в кодовой базе. Добавлена ранняя проверка: если clamd не запущен — возврат без запуска `ps`.

### 25.4. Symlink attack в writeSetupFile()

| Было | Стало |
|------|-------|
| `file.remove()` → `QFile::open(WriteOnly)` | `QFile::open(WriteOnly \| Truncate \| Text)` |

Между `remove()` и `open()` злоумышленник мог подменить путь на symlink. `Truncate` атомарно очищает существующий файл.

### 25.5. Бесконечный цикл при disk-full

| Было | Стало |
|------|-------|
| `do { } while (!flush());` | `if (!flush()) qWarning(...) ;` |

При переполнении диска `flush()` возвращал `false` вечно → зависание приложения.

### 25.6. restartClamonacc() убивал clamd вместо clamonacc

Функция «перезапуска on-access» фактически останавливала **clamd** (фоновый демон), а не **clamonacc**. Исправлена логика: SIGTERM → clamonacc, перезапуск clamonacc если настроены monitored directories.

### 25.7. Missing PID validation в freshclamsetter.cpp stop-path

Добавлена проверка `isValidPid()` перед `kill`. Устранена возможность отправки сигнала произвольному процессу из-за race condition (PID reuse).

### 25.8. SIGKILL → SIGTERM для graceful shutdown

**`clamdmanager.cpp`** (2 места): `kill -9` заменён на `kill -TERM`. SIGKILL мог повредить базы данных ClamAV при незавершённой записи.

### 25.9. QMovie memory leak

**`clamdmanager.cpp`** (2 места): `new QMovie(":/icons/...")` без parent → утечка при каждом создании/уничтожении виджета. Добавлен `this` как parent.

### 25.10. Cross-call state corruption (static local)

**`freshclamsetter.cpp/.h`**: `static QString oldLine` внутри функции сохранял состояние между вызовами разных потоков/инстансов. Заменён на член класса `m_oldLine`, инициализируемый `clear()` в начале функции.

### 25.11. ODR violation: sharedvars.cpp includится из 7 .cpp файлов

| Было | Стало |
|------|-------|
| `#include "sharedvars.cpp"` в 7 файлах | `#include "sharedvars.h"` (новый файл с include guard) |

Переменные `const QStringList` и enum'ы перенесены в `sharedvars.h`. Файл `sharedvars.cpp` оставлен пустым для совместимости с qmake build system.

### 25.12. Улучшение build system (CMakeLists.txt)

| Изменение | Описание |
|-----------|----------|
| C++11 → C++17 | Qt6 требует C++17; добавлен `CMAKE_CXX_STANDARD_REQUIRED ON` |
| `-Wall -Wextra -Wpedantic` | Обнаружение скрытых дефектов на этапе компиляции |
| `CONFIGURE_DEPENDS` на GLOB | Автоматический reconfigure при добавлении/удалении файлов |
| Удалены `message(STATUS ...)` debug-строки | Чистый вывод cmake |

---

## Итоговая статистика

| Категория | Найдено | Исправлено |
|-----------|---------|------------|
| Опечатки в коде | 7 типов (≈50 вхождений) | ✅ Все |
| Опечатки в .ts переводах | 4 типа (54 вхождения, 9 файлов) | ✅ Все |
| Command Injection | 7 | ✅ Все |
| Insecure Permissions | 3 | ✅ Все |
| Path Traversal | 2 | ✅ Все |
| Race Conditions | 4 | ✅ Все |
| Выбор языка не работает (dev mode) | 3 причины | ✅ Все |
| Файлы .qm устарели | 20 | ✅ Пересобраны |
| Непереведённые строки UI (секция 7) | 6 (4 строки + 2 таб-заголовка) | ✅ Все |
| Непереведённые строки UI (секция 15) | 14 строк (7 файлов) | ✅ Все |
| Логический баг: "not Running" vs "not running" | 1 | ✅ Исправлен |
| Man-страница ru отсутствует | 1 | ✅ Создана |
| Кнопки с обрезанным текстом | 3 | ✅ Увеличены до 180 px |
| «Direct Scan» не переведён | 3 места | ✅ Все |
| Метка «Имя профиля» обрезается | 1 | ✅ Ширина 150 px |
| HTML-блок приветствия не переводится | 1 | ✅ Исправлен source |
| Метка «Папка под наблюдением» обрезается | 1 | ✅ Ширина 200 px |
| U+2010 в source-строках .ts (неперевод) | Все 10 файлов (39–56 вхождений каждый) | ✅ Удалены из .ts |
| U+2010 в рантайме (man page → label) | 4 параметра | ✅ Нормализация в коде |
| Обрезанный source (CacheSize) | 1 | ✅ Дополнен |
| Дубликат записи (PUA category) | 1 | ✅ Удалён |
| Несоответствие пути /etc/certs | Все 10 файлов | ✅ → /etc/clamav/certs |
| AppImage: бинарник в `bin/`, переводы в `usr/share/` | 1 | ✅ → `usr/bin/` |
| Дубликаты установки в пакетах (install.cmake) | 6 правил | ✅ Удалены |
| «yes»/«no» в combobox на вкладке Clamd не переведены | 2 значения | ✅ → «да»/«нет» |
| AppImage: иконка в .desktop не совпадает с файлом | 1 | ✅ `Icon=clamav-gui` |
| Переведённые «да»/«нет» записываются в конфиги (scanoptionyn, freshclamsetter, clamdconfcomboboxoption) | 3 файла, 10+ мест | ✅ itemData pattern |
| AppImage: выбор языка во вкладке «Настройки» недоступен | 1 (setuptab.cpp) | ✅ Удалён неверный путь |
| Пакет для Arch Linux отсутствует | 1 | ✅ PKGBUILD + build-local.sh |
| Dangling pointer UB (setupfilehandler) | 1 | ✅ Исправлен |
| clamonacc опции одним аргументом | 2 места | ✅ split() |
| Последний bash-shell в коде (findClamdProcess) | 1 | ✅ pidof + ps |
| Symlink attack (writeSetupFile) | 1 | ✅ Truncate |
| Бесконечный цикл (flush) | 1 | ✅ if + qWarning |
| restartClamonacc убивал clamd | 1 | ✅ Логика исправлена |
| Missing PID validation (freshclam stop) | 1 | ✅ isValidPid() |
| SIGKILL → SIGTERM | 2 места | ✅ kill -TERM |
| QMovie memory leak | 2 места | ✅ parent=this |
| Static local state corruption | 1 | ✅ m_oldLine member |
| ODR violation (sharedvars.cpp ×7) | 7 файлов | ✅ sharedvars.h |
| C++ standard / warnings / GLOB (CMakeLists) | 4 пункта | ✅ C++17, -Wall, CONFIGURE_DEPENDS |
