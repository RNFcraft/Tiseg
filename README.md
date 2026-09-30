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

## Пример: таймер 0 → 60 через `for`

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
            // Во время счёта ведущие нули выключены.
            display.printR(seconds, false);

            // Конец диапазона задаётся прямо циклом for.
            if (seconds == 60) {
                timer.pause();
                break;
            }

            // Ждём секунду, но дисплей и кнопка продолжают работать.
            // При нажатии кнопки wait() вернёт false и цикл остановится.
            if (!timer.wait(1000, display)) {
                break;
            }
        }
    }

    if (timer.justPaused()) {
        // На паузе ведущие нули включены.
        display.printR(seconds, true);
    }

    if (timer.justReset()) {
        seconds = 0;
        display.printR(0, true); // 0000
    }
}
```

### Поведение примера

- после включения: `0000`;
- первое нажатие — запуск: `___0`, `___1`, `___2` ... `__12` ...;
- второе нажатие во время счёта — пауза и ведущие нули, например `0012`;
- третье нажатие — сброс в `0000`;
- если цикл сам доходит до `60`, он останавливается на `0060`.

Главное преимущество — сам таймер теперь задаётся обычным `for`:

```cpp
for (seconds = 0; seconds <= 60; seconds++) {
    ...
}
```

Чтобы считать до 120:

```cpp
for (seconds = 0; seconds <= 120; seconds++) {
    ...
}
```

Чтобы считать с 10 до 30:

```cpp
for (seconds = 10; seconds <= 30; seconds++) {
    ...
}
```

Чтобы сделать обратный отсчёт:

```cpp
for (seconds = 60; seconds >= 0; seconds--) {
    ...
}
```

Шаг времени тоже остаётся в скетче:

```cpp
timer.wait(500, display);  // шаг каждые 0.5 секунды
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

Для таймера через `for` используется:

```cpp
timer.wait(milliseconds, display);
```

`wait()` намеренно удерживает выполнение внутри текущего цикла `for`, но при этом постоянно вызывает `display.tick()` и `timer.tick()`. Поэтому динамическая индикация не останавливается, а кнопка остаётся рабочей.

Если во время `wait()` нажать кнопку, состояние меняется на `PAUSED`, `wait()` возвращает `false`, и цикл можно немедленно прервать:

```cpp
if (!timer.wait(1000, display)) {
    break;
}
```

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

Для более сложных проектов можно вообще не использовать `wait()` и работать с `tick()`/состояниями неблокирующим способом.

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
