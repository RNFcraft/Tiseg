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
- `TisegTimerControl` — обвязка кнопки для цикла **start → pause → reset**.

`TisegTimerControl` специально **не считает время и не управляет дисплеем**. Секунды, предел таймера, формат вывода и вся прикладная логика остаются в скетче, чтобы её можно было легко менять.

## Важно: общий анод

Текущая реализация рассчитана на **прямое подключение общего анода разряда к пину Arduino**:

- общий анод разряда: `HIGH` — разряд включён, `LOW` — выключен;
- сегменты A..G/DP: `LOW` — сегмент горит, `HIGH` — выключен.

При мультиплексировании библиотека сначала выключает все разряды, затем меняет состояние сегментов и только после этого включает один нужный разряд. Это предотвращает паразитное отображение цифры на соседних разрядах.

Если общие аноды управляются через инвертирующий транзисторный каскад, полярность управляющих пинов будет другой.

## Пример: редактируемый таймер 0 → 60

```cpp
#include <Tiseg.h>

const uint8_t digitPins[] = {13, 12, 11, 10};
const uint8_t segmentPins[] = {2, 3, 4, 5, 6, 7, 8, 1};

const uint8_t BUTTON_PIN = 9;
const unsigned long MAX_SECONDS = 60;

Tiseg<4> display(digitPins, segmentPins);
TisegTimerControl timer(BUTTON_PIN);

unsigned long seconds = 0;
unsigned long lastSecondAt = 0;

void setup() {
    display.begin();
    timer.begin();
    display.printR(0, true); // 0000
}

void loop() {
    display.tick();
    timer.tick();

    if (timer.justStarted()) {
        lastSecondAt = millis();
        display.printR((long)seconds, false);
    }

    if (timer.justPaused()) {
        display.printR((long)seconds, true);
    }

    if (timer.justReset()) {
        seconds = 0;
        display.printR(0, true);
    }

    if (timer.isRunning()) {
        unsigned long now = millis();

        while (timer.isRunning() && now - lastSecondAt >= 1000UL) {
            lastSecondAt += 1000UL;
            seconds++;
            display.printR((long)seconds, false);

            if (seconds >= MAX_SECONDS) {
                seconds = MAX_SECONDS;
                timer.pause();
                display.printR((long)seconds, true);
            }
        }
    }
}
```

### Поведение этого примера

- после включения: `0000`;
- первое нажатие — запуск, во время счёта: `___0`, `___1`, `__12` ...;
- второе нажатие — пауза и заполнение нулями: `0012`;
- третье нажатие — сброс: `0000`;
- при достижении `60` пример сам вызывает `timer.pause()` и показывает `0060`.

При этом всё легко менять прямо в скетче. Например:

```cpp
const unsigned long MAX_SECONDS = 120;
```

или убрать автоматическую остановку, изменить шаг времени, формат вывода, добавить мигание и т.д.

## `TisegTimerControl`

Создание:

```cpp
TisegTimerControl timer(buttonPin);
```

В `setup()`:

```cpp
timer.begin();
```

В `loop()`:

```cpp
timer.tick();
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

Состоянием можно управлять вручную из своего кода:

```cpp
timer.start();
timer.pause();
timer.reset();
```

То есть библиотека убирает только повторяющийся код кнопки и переключения состояний, но не прячет сам таймер.

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
