# Tiseg — 7-сегментный дисплей и удобная обвязка таймера для Arduino

[![Platform](https://img.shields.io/badge/platform-Arduino-blue)](https://www.arduino.cc)
[![Dependencies](https://img.shields.io/badge/dependencies-none-green)](#установка)

Для всей библиотеки достаточно одного подключения:

```cpp
#include <Tiseg.h>
```

После этого доступны:

- `Tiseg<DIGITS>` — вывод чисел на дисплей;
- `TisegButton` — отдельная кнопка с антидребезгом;
- `TisegTimerControl` — обвязка кнопки для цикла **start → pause → reset** и удобного таймера через обычный `for`.

`TisegTimerControl` не задаёт длительность таймера и не хранит максимальное число секунд. Диапазон задаётся прямо в скетче обычным циклом `for`, поэтому таймер легко менять без правки библиотеки.

## Важно: общий анод

Текущая реализация рассчитана на **прямое подключение общего анода разряда к пину Arduino**:

- общий анод разряда: `HIGH` — разряд включён, `LOW` — выключен;
- сегменты A..G/DP: `LOW` — сегмент горит, `HIGH` — выключен.

При мультиплексировании библиотека сначала выключает все разряды, затем меняет состояние сегментов и только после этого включает один нужный разряд. Это предотвращает паразитное отображение цифры на соседних разрядах.

Если общие аноды управляются через инвертирующий транзисторный каскад, полярность управляющих пинов будет другой.

## Пример: таймер 0 → 60 через `for` с миганием

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
            // Новое число сразу появляется на дисплее.
            display.printR(seconds, false);

            if (seconds == 60) {
                timer.pause();
                break;
            }

            // Первая половина секунды: число видно.
            // Вторая половина: дисплей полностью погашен.
            // После этого for переключает seconds и следующее printR()
            // зажигает уже новое число.
            if (!timer.waitBlink(1000, display)) {
                break;
            }
        }
    }

    if (timer.justPaused()) {
        // На паузе дисплей постоянно включён и ведущие нули видны.
        display.printR(seconds, true);
    }

    if (timer.justReset()) {
        seconds = 0;
        display.printR(0, true); // 0000
    }
}
```

### Как выглядит один шаг

Для `timer.waitBlink(1000, display)`:

```text
___1  — 500 мс
____  — 500 мс
___2  — 500 мс
____  — 500 мс
___3  — 500 мс
...
```

То есть дисплей не зажигает старое число после тёмной половины. Он остаётся погашенным до следующего `display.printR()`, которое сначала записывает новое значение и только затем снова включает индикацию.

### Поведение примера

- после включения: `0000`;
- первое нажатие — запуск и мигающий счёт без ведущих нулей;
- второе нажатие во время счёта — пауза, дисплей остаётся включённым и показывает, например, `0012`;
- третье нажатие — сброс в `0000`;
- если цикл сам доходит до `60`, он останавливается на `0060`.

Сам диапазон по-прежнему задаётся обычным `for`:

```cpp
for (seconds = 0; seconds <= 60; seconds++) {
    ...
}
```

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

## `TisegTimerControl`

Создание:

```cpp
TisegTimerControl timer(buttonPin);
```

В `setup()`:

```cpp
timer.begin();
```

В обычном `loop()`:

```cpp
timer.tick();
```

### Обычное ожидание

Если мигание не нужно:

```cpp
if (!timer.wait(1000, display)) {
    break;
}
```

`wait()` удерживает выполнение внутри текущего цикла `for`, но постоянно вызывает `display.tick()` и `timer.tick()`, поэтому дисплей и кнопка продолжают работать.

### Ожидание с миганием

```cpp
if (!timer.waitBlink(1000, display)) {
    break;
}
```

`waitBlink()` делит интервал пополам:

- первая половина — дисплей включён;
- вторая половина — дисплей выключен;
- по окончании функция оставляет дисплей погашенным до следующего `print()`/`printR()`/`printL()`;
- если во время ожидания нажать паузу, дисплей снова разрешается и функция возвращает `false`.

Например:

```cpp
timer.waitBlink(2000, display);
```

означает 1 секунду отображения и 1 секунду погашенного дисплея.

Состояния:

```cpp
timer.isReady();
timer.isRunning();
timer.isPaused();
```

Одноразовые события переходов:

```cpp
timer.justStarted();
timer.justPaused();
timer.justReset();
```

Состоянием можно управлять вручную:

```cpp
timer.start();
timer.pause();
timer.reset();
```

Для более сложных проектов можно вообще не использовать `wait()`/`waitBlink()` и работать с `tick()`/состояниями неблокирующим способом.

## Вывод чисел через `Tiseg`

```cpp
Tiseg<4> display(digitPins, segmentPins);
```

Основные методы:

```cpp
display.begin();
display.tick();

display.printR(1);        // ___1
display.printR(12);       // __12
display.printR(12, true); // 0012

display.printL(1);        // 1___
display.printL(12);       // 12__
display.printL(12, true); // 1200

display.print(123);       // _123
display.clear();
```

Для временного погашения дисплея без потери числа:

```cpp
display.hide();
display.show();
display.setEnabled(false);
display.setEnabled(true);
```

`print()`, `printR()` и `printL()` автоматически включают дисплей снова.

## `TisegButton`

Для произвольной работы с кнопкой:

```cpp
TisegButton button(9);
```

Доступны:

```cpp
button.begin();
button.tick();
button.onPress(myFunction);
button.onRelease(myFunction);
button.isPressed();
button.wasPressed();
button.wasReleased();
```

По умолчанию используется `INPUT_PULLUP`, поэтому кнопка подключается между Arduino-пином и `GND`.

## Установка

Скопируйте папку `Tiseg` в `Documents/Arduino/libraries` или установите ZIP через Arduino IDE.

Внешних зависимостей нет.

## Структура

```text
Tiseg/
├── Tiseg.h
├── TisegButton.h
├── TisegTimerControl.h
├── library.properties
└── examples/
    ├── BasicDisplay/
    │   └── BasicDisplay.ino
    └── ButtonTimer/
        └── ButtonTimer.ino
```
