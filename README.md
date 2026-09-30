# Tiseg — 7-сегментный дисплей, кнопка и таймер для Arduino

[![Platform](https://img.shields.io/badge/platform-Arduino-blue)](https://www.arduino.cc)
[![Dependencies](https://img.shields.io/badge/dependencies-none-green)](#установка)

`Tiseg` — лёгкая библиотека для мультиплексируемых 7-сегментных индикаторов с общим анодом. Она умеет выводить числа, работать с кнопкой с антидребезгом и упрощать создание редактируемых таймеров через обычный `for`.

Для всей библиотеки достаточно одного подключения:

```cpp
#include <Tiseg.h>
```

После этого доступны три класса:

- `Tiseg<DIGITS>` — управление дисплеем;
- `TisegButton` — отдельная кнопка с антидребезгом;
- `TisegTimerControl` — управление состояниями таймера `start → pause → reset`.

---

## Содержание

- [Подключение дисплея](#подключение-дисплея)
- [Быстрый пример](#быстрый-пример)
- [Tiseg — дисплей](#tiseg--дисплей)
- [TisegButton — кнопка](#tisegbutton--кнопка)
- [TisegTimerControl — таймер](#tisegtimercontrol--таймер)
- [Готовый таймер через for](#готовый-таймер-через-for)
- [Мигание таймера](#мигание-таймера)
- [Готовые примеры](#готовые-примеры)
- [Установка](#установка)

---

# Подключение дисплея

Текущая реализация рассчитана на **прямое подключение общего анода каждого разряда к пину Arduino**.

Полярность:

```text
общий анод разряда:
HIGH = разряд включён
LOW  = разряд выключен

сегменты A..G/DP:
LOW  = сегмент горит
HIGH = сегмент выключен
```

Пример подключения четырёхразрядного индикатора:

```text
Разряд 1 -> 13
Разряд 2 -> 12
Разряд 3 -> 11
Разряд 4 -> 10

A  -> 2
B  -> 3
C  -> 4
D  -> 5
E  -> 6
F  -> 7
G  -> 8
DP -> 1

Кнопка -> 9 и GND
```

Если общие аноды управляются через инвертирующий транзисторный каскад, эта полярность не подходит без изменения драйвера.

---

# Быстрый пример

```cpp
#include <Tiseg.h>

const uint8_t digitPins[] = {13, 12, 11, 10};
const uint8_t segmentPins[] = {2, 3, 4, 5, 6, 7, 8, 1};

Tiseg<4> display(digitPins, segmentPins);

void setup() {
    display.begin();
    display.printR(123);
}

void loop() {
    display.tick();
}
```

На четырёхразрядном индикаторе получится:

```text
_123
```

`display.tick()` нужно вызывать как можно чаще, потому что индикатор работает через динамическую индикацию.

---

# `Tiseg` — дисплей

## Создание объекта

Синтаксис:

```cpp
Tiseg<DIGITS> display(digitPins, segmentPins);
```

Пример:

```cpp
const uint8_t digitPins[] = {13, 12, 11, 10};
const uint8_t segmentPins[] = {2, 3, 4, 5, 6, 7, 8, 1};

Tiseg<4> display(digitPins, segmentPins);
```

Параметры:

- `DIGITS` — количество разрядов индикатора;
- `digitPins` — массив пинов общих анодов, ровно `DIGITS` элементов;
- `segmentPins` — массив из 8 пинов в порядке `A, B, C, D, E, F, G, DP`.

---

## `begin()`

Инициализирует пины дисплея.

```cpp
display.begin();
```

Вызывается один раз в `setup()`.

---

## `tick()`

Обновляет динамическую индикацию.

```cpp
display.tick();
```

Должен вызываться как можно чаще в `loop()` или внутри функций вроде `timer.wait()` / `timer.waitBlink()`.

---

## `print()`

Сокращение для правого выравнивания.

Синтаксис:

```cpp
display.print(number);
display.print(number, fillZeros);
```

Параметры:

- `number` — число типа `long`;
- `fillZeros` — заполнять ли свободные разряды нулями, по умолчанию `false`.

Примеры:

```cpp
display.print(1);        // ___1
display.print(12);       // __12
display.print(12, true); // 0012
```

`print()` автоматически включает дисплей, если он был выключен через `hide()`.

---

## `printR()`

Выводит число с выравниванием вправо.

Синтаксис:

```cpp
display.printR(number);
display.printR(number, fillZeros);
```

Примеры для 4 разрядов:

```cpp
display.printR(1);        // ___1
display.printR(12);       // __12
display.printR(123);      // _123
display.printR(12, true); // 0012
display.printR(0, true);  // 0000
```

Отрицательные значения поддерживаются:

```cpp
display.printR(-12); // _-12
```

Если число длиннее доступного количества разрядов, старшие лишние цифры не отображаются.

---

## `printL()`

Выводит число с выравниванием влево.

Синтаксис:

```cpp
display.printL(number);
display.printL(number, fillZeros);
```

Примеры:

```cpp
display.printL(1);        // 1___
display.printL(12);       // 12__
display.printL(12, true); // 1200
```

---

## `clear()`

Очищает буфер дисплея.

```cpp
display.clear();
```

После этого все разряды становятся пустыми.

`clear()` очищает содержимое буфера, а `hide()` только временно выключает физический индикатор.

---

## `hide()`

Временно гасит дисплей без очистки буфера.

```cpp
display.hide();
```

Например, если до этого было `_123`, после `hide()` дисплей станет пустым, но значение `_123` останется в памяти.

---

## `show()`

Снова разрешает отображение содержимого буфера.

```cpp
display.show();
```

---

## `setEnabled()`

Явное включение или выключение дисплея.

Синтаксис:

```cpp
display.setEnabled(true);
display.setEnabled(false);
```

Эквиваленты:

```cpp
display.show(); // setEnabled(true)
display.hide(); // setEnabled(false)
```

---

## `isEnabled()`

Возвращает состояние физического отображения.

```cpp
bool enabled = display.isEnabled();
```

Возвращает:

- `true` — дисплей разрешён;
- `false` — дисплей погашен.

---

# `TisegButton` — кнопка

`TisegButton` доступен через тот же:

```cpp
#include <Tiseg.h>
```

Отдельный `#include <TisegButton.h>` не нужен.

По умолчанию используется `INPUT_PULLUP`:

```text
Arduino pin ---- кнопка ---- GND
```

---

## Создание кнопки

Полный синтаксис:

```cpp
TisegButton button(pin, debounceMs, activeLow);
```

Все параметры кроме `pin` необязательны.

Простой вариант:

```cpp
TisegButton button(9);
```

Полный вариант:

```cpp
TisegButton button(9, 50, true);
```

Параметры:

- `pin` — пин кнопки;
- `debounceMs` — время антидребезга в миллисекундах, по умолчанию `50`;
- `activeLow` — логика кнопки:
  - `true` — используется `INPUT_PULLUP`, нажатие = `LOW`;
  - `false` — используется обычный `INPUT`, нажатие = `HIGH`.

---

## `begin()`

Настраивает пин кнопки.

```cpp
button.begin();
```

Вызывается в `setup()`.

---

## `tick()`

Обновляет состояние кнопки и выполняет антидребезг.

```cpp
button.tick();
```

Нужно вызывать как можно чаще в `loop()`.

---

## `onPress()`

Назначает функцию, которая вызывается один раз при нажатии.

Синтаксис:

```cpp
button.onPress(function);
```

Пример:

```cpp
void pressed() {
    // действие
}

void setup() {
    button.begin();
    button.onPress(pressed);
}
```

Callback должен иметь вид:

```cpp
void function();
```

---

## `onRelease()`

Назначает функцию, которая вызывается один раз при отпускании.

```cpp
button.onRelease(function);
```

Пример:

```cpp
void released() {
    // действие
}
```

---

## `isPressed()`

Возвращает текущее состояние кнопки после антидребезга.

```cpp
if (button.isPressed()) {
    // кнопка сейчас удерживается
}
```

---

## `wasPressed()`

Возвращает `true` один раз после каждого нажатия.

```cpp
if (button.wasPressed()) {
    // одно событие нажатия
}
```

После чтения флаг события сбрасывается.

---

## `wasReleased()`

Возвращает `true` один раз после каждого отпускания.

```cpp
if (button.wasReleased()) {
    // одно событие отпускания
}
```

После чтения флаг события сбрасывается.

---

# `TisegTimerControl` — таймер

`TisegTimerControl` **не считает секунды сам** и не хранит максимальную длительность.

Он только:

- обрабатывает кнопку;
- хранит состояние таймера;
- переключает `READY → RUNNING → PAUSED → READY`;
- выдаёт одноразовые события;
- предоставляет `wait()` и `waitBlink()` для простых циклов `for`.

Сам счётчик остаётся обычным кодом пользователя.

---

## Создание

Полный синтаксис:

```cpp
TisegTimerControl timer(buttonPin, debounceMs, activeLow);
```

Простой вариант:

```cpp
TisegTimerControl timer(9);
```

Параметры:

- `buttonPin` — пин кнопки;
- `debounceMs` — антидребезг, по умолчанию `50` мс;
- `activeLow` — `true` для стандартной схемы `pin -> button -> GND`.

---

## Состояния

Доступны три состояния:

```cpp
TisegTimerControl::READY
TisegTimerControl::RUNNING
TisegTimerControl::PAUSED
```

Обычный цикл кнопки:

```text
READY
  ↓ первое нажатие
RUNNING
  ↓ второе нажатие
PAUSED
  ↓ третье нажатие
READY
```

---

## `begin()`

Инициализирует кнопку и устанавливает состояние `READY`.

```cpp
timer.begin();
```

---

## `tick()`

Опрос кнопки и обновление состояний.

```cpp
timer.tick();
```

В обычном неблокирующем коде вызывается в `loop()`.

Внутри `wait()` и `waitBlink()` он вызывается автоматически.

---

## `start()`

Переводит таймер в состояние `RUNNING`.

```cpp
timer.start();
```

Также создаёт событие `justStarted()`.

---

## `pause()`

Переводит таймер из `RUNNING` в `PAUSED`.

```cpp
timer.pause();
```

Также создаёт событие `justPaused()`.

Если таймер не находится в `RUNNING`, вызов ничего не делает.

---

## `reset()`

Возвращает состояние `READY`.

```cpp
timer.reset();
```

Также создаёт событие `justReset()`.

Важно: `reset()` не меняет вашу переменную `seconds`. Её нужно сбросить самостоятельно:

```cpp
seconds = 0;
timer.reset();
```

---

## `state()`

Возвращает текущее состояние.

```cpp
TisegTimerControl::State current = timer.state();
```

Пример:

```cpp
if (timer.state() == TisegTimerControl::RUNNING) {
    // таймер работает
}
```

---

## `isReady()`

```cpp
if (timer.isReady()) {
    // READY
}
```

Возвращает `true`, если состояние `READY`.

---

## `isRunning()`

```cpp
if (timer.isRunning()) {
    // RUNNING
}
```

Возвращает `true`, если таймер работает.

---

## `isPaused()`

```cpp
if (timer.isPaused()) {
    // PAUSED
}
```

Возвращает `true`, если таймер на паузе.

---

## `justStarted()`

Одноразовое событие перехода в `RUNNING`.

```cpp
if (timer.justStarted()) {
    // выполняется один раз после старта
}
```

При первом чтении возвращает `true`, затем событие сбрасывается.

---

## `justPaused()`

Одноразовое событие перехода в `PAUSED`.

```cpp
if (timer.justPaused()) {
    display.printR(seconds, true);
}
```

Удобно для изменения отображения при остановке.

---

## `justReset()`

Одноразовое событие сброса в `READY`.

```cpp
if (timer.justReset()) {
    seconds = 0;
    display.printR(0, true);
}
```

---

## `wait()`

Ждёт указанный интервал, но продолжает обслуживать дисплей и кнопку.

Синтаксис:

```cpp
bool result = timer.wait(durationMs, display);
```

Пример:

```cpp
if (!timer.wait(1000, display)) {
    break;
}
```

Параметры:

- `durationMs` — длительность ожидания в миллисекундах;
- `display` — объект с методом `tick()`, обычно `Tiseg`.

Во время ожидания функция постоянно вызывает:

```cpp
display.tick();
timer.tick();
```

Возвращает:

- `true` — весь интервал завершён и таймер всё ещё `RUNNING`;
- `false` — во время ожидания таймер вышел из `RUNNING`, например пользователь нажал кнопку паузы.

Пример в `for`:

```cpp
for (int seconds = 0; seconds <= 60; seconds++) {
    display.printR(seconds);

    if (!timer.wait(1000, display)) {
        break;
    }
}
```

---

## `waitBlink()`

То же ожидание, но с миганием дисплея.

Синтаксис:

```cpp
bool result = timer.waitBlink(durationMs, display);
```

Пример:

```cpp
if (!timer.waitBlink(1000, display)) {
    break;
}
```

Для `1000` мс один шаг выглядит так:

```text
0-500 мс     число видно
500-1000 мс  дисплей погашен
после 1000   for переключает число
```

Например:

```text
___1 -> ____ -> ___2 -> ____ -> ___3
```

На нормальном завершении `waitBlink()` оставляет дисплей погашенным. Следующий вызов `print()`, `printR()` или `printL()` сначала записывает новое число, затем включает дисплей. Поэтому старое число не вспыхивает повторно.

Если во время ожидания нажать кнопку:

- таймер переходит в `PAUSED`;
- дисплей снова включается;
- функция возвращает `false`;
- цикл можно сразу остановить.

Другие интервалы:

```cpp
timer.waitBlink(500, display);  // 250 мс видно + 250 мс темно
timer.waitBlink(2000, display); // 1 с видно + 1 с темно
```

---

## `button()`

Возвращает внутренний `TisegButton`, если нужен более низкоуровневый доступ.

```cpp
TisegButton& button = timer.button();
```

Пример:

```cpp
if (timer.button().isPressed()) {
    // кнопка удерживается
}
```

---

# Готовый таймер через `for`

Полный пример для пинов:

- общие аноды: `13, 12, 11, 10`;
- кнопка: `9`;
- сегменты `A..G`: `2, 3, 4, 5, 6, 7, 8`;
- `DP`: `1`.

```cpp
#include <Tiseg.h>

const uint8_t digitPins[] = {13, 12, 11, 10};
const uint8_t segmentPins[] = {2, 3, 4, 5, 6, 7, 8, 1};

Tiseg<4> display(digitPins, segmentPins);
TisegTimerControl timer(9);

int seconds = 0;

void setup() {
    display.begin();
    timer.begin();

    display.printR(0, true); // 0000
}

void loop() {
    display.tick();
    timer.tick();

    if (timer.justStarted()) {
        for (seconds = 0; seconds <= 60; seconds++) {
            display.printR(seconds, false);

            if (seconds == 60) {
                timer.pause();
                break;
            }

            if (!timer.waitBlink(1000, display)) {
                break;
            }
        }
    }

    if (timer.justPaused()) {
        display.printR(seconds, true);
    }

    if (timer.justReset()) {
        seconds = 0;
        display.printR(0, true);
    }
}
```

Поведение:

```text
До запуска:
0000

Во время счёта:
___0 -> темно -> ___1 -> темно -> ___2 ...

Пауза:
например 0012

Сброс:
0000

Достижение 60:
0060
```

---

# Мигание таймера

Чтобы использовать обычный шаг без мигания:

```cpp
timer.wait(1000, display);
```

Чтобы во второй половине шага дисплей гас:

```cpp
timer.waitBlink(1000, display);
```

Смена диапазона не требует изменений библиотеки.

До 120:

```cpp
for (seconds = 0; seconds <= 120; seconds++) {
    ...
}
```

С 10 до 30:

```cpp
for (seconds = 10; seconds <= 30; seconds++) {
    ...
}
```

Обратный отсчёт:

```cpp
for (seconds = 60; seconds >= 0; seconds--) {
    ...
}
```

При обратном отсчёте следите, чтобы переменная была знакового типа (`int` или `long`), иначе unsigned-переменная после нуля переполнится.

---

# Краткая таблица API

## `Tiseg<DIGITS>`

| Метод | Назначение |
|---|---|
| `begin()` | Инициализация пинов дисплея |
| `tick()` | Обновление мультиплексирования |
| `print(num, fillZeros)` | Вывод числа справа |
| `printR(num, fillZeros)` | Вывод справа |
| `printL(num, fillZeros)` | Вывод слева |
| `clear()` | Очистка буфера |
| `hide()` | Погасить дисплей без очистки |
| `show()` | Снова включить дисплей |
| `setEnabled(bool)` | Включить/выключить отображение |
| `isEnabled()` | Узнать состояние отображения |

## `TisegButton`

| Метод | Назначение |
|---|---|
| `begin()` | Инициализация кнопки |
| `tick()` | Опрос и антидребезг |
| `onPress(fn)` | Callback на нажатие |
| `onRelease(fn)` | Callback на отпускание |
| `isPressed()` | Кнопка сейчас нажата |
| `wasPressed()` | Одноразовое событие нажатия |
| `wasReleased()` | Одноразовое событие отпускания |

## `TisegTimerControl`

| Метод | Назначение |
|---|---|
| `begin()` | Инициализация и состояние READY |
| `tick()` | Опрос кнопки и переключение состояний |
| `start()` | Перейти в RUNNING |
| `pause()` | Перейти в PAUSED |
| `reset()` | Вернуться в READY |
| `state()` | Получить текущее состояние |
| `isReady()` | Проверить READY |
| `isRunning()` | Проверить RUNNING |
| `isPaused()` | Проверить PAUSED |
| `justStarted()` | Одно событие старта |
| `justPaused()` | Одно событие паузы |
| `justReset()` | Одно событие сброса |
| `wait(ms, display)` | Ожидание с работающими дисплеем и кнопкой |
| `waitBlink(ms, display)` | Ожидание с гашением второй половины интервала |
| `button()` | Доступ к внутреннему `TisegButton` |

---

# Готовые примеры

После установки библиотеки примеры должны появиться в Arduino IDE через:

```text
File -> Examples -> Tiseg
```

В библиотеке есть:

```text
BasicDisplay
ButtonTimer
TimerTemplate
```

### `BasicDisplay`

Минимальный пример обычного вывода чисел.

### `ButtonTimer`

Пример таймера с кнопкой и текущей логикой библиотеки.

### `TimerTemplate`

Готовый шаблон для нового проекта:

- уже содержит пины `13,12,11,10` для общих анодов;
- кнопку на пине `9`;
- `A..G` на `2..8`;
- `DP` на `1`;
- таймер `0..60` через `for`;
- мигание 500/500 мс;
- start / pause / reset;
- комментарии, где менять диапазон и скорость.

---

# Установка

## Через ZIP

1. Скачайте библиотеку ZIP.
2. В Arduino IDE откройте:

```text
Sketch -> Include Library -> Add .ZIP Library...
```

3. Выберите ZIP.

## Вручную

Скопируйте папку `Tiseg` в:

```text
Documents/Arduino/libraries/
```

После установки достаточно:

```cpp
#include <Tiseg.h>
```

Внешних зависимостей нет.

---

# Структура библиотеки

```text
Tiseg/
├── Tiseg.h
├── TisegButton.h
├── TisegTimerControl.h
├── library.properties
└── examples/
    ├── BasicDisplay/
    │   └── BasicDisplay.ino
    ├── ButtonTimer/
    │   └── ButtonTimer.ino
    └── TimerTemplate/
        └── TimerTemplate.ino
```
