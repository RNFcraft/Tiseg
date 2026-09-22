#pragma once
#include <Arduino.h>
#include <GyverSegment.h>

/**
 * Класс Tiseg для управления таймером на семисегментном дисплее.
 * Шаблонный параметр DIGITS задает количество разрядов (например, 4).
 */
template <uint8_t DIGITS>
class Tiseg {
  public:
    /**
     * Конструктор класса
     * @param digitPins   Массив пинов разрядов дисплея
     * @param segmentPins Массив пинов сегментов дисплея
     * @param buttonPin   Пин подключения кнопки
     */
    Tiseg(const uint8_t* digitPins, const uint8_t* segmentPins, uint8_t buttonPin) :
        _disp(digitPins, segmentPins), _btn(buttonPin) {}

    /**
     * Инициализация периферии. Вызывать внутри setup().
     */
    void begin() {
        pinMode(_btn, INPUT_PULLUP);
        showZero();
        _disp.delay(2000); // Стартовая задержка, как в исходном коде
    }

    /**
     * Обновление динамической индикации дисплея. 
     * Необходимо постоянно вызывать внутри главного loop().
     */
    void tick() {
        _disp.tick();
    }

    /**
     * Проверка нажатия кнопки (LOW-уровень) с защитой от дребезга.
     */
    bool isButtonPressed() {
        if (digitalRead(_btn) == LOW) {
            delay(50); // Базовый антидребезг
            if (digitalRead(_btn) == LOW) {
                return true;
            }
        }
        return false;
    }

    /**
     * Вывод "0000" (или соответствующего числа нулей) на дисплей.
     */
    void showZero() {
        _disp.clearPrintR("0000");
        _disp.update();
    }

    /**
     * Запуск цикла таймера от 0 до указанного числа секунд.
     * @param maxSeconds Время, до которого идет отсчет (например, 60).
     */
    void runTimer(int maxSeconds) {
        for (int num = 0; num <= maxSeconds; num++) {
            _disp.clearPrintR(num);
            _disp.update();
            
            bool blinked = false; // Флаг: моргали ли уже в этой секунде
            unsigned long startTime = millis();

            // Внутренний цикл удержания одной секунды
            while (millis() - startTime < 1000) {
                _disp.tick(); // Постоянное обновление дисплея во время ожидания
                unsigned long elapsed = millis() - startTime;

                // Спустя 500 мс тушим экран для эффекта мигания
                if (elapsed >= 500 && !blinked) {
                    _disp.clear();
                    _disp.update();
                    blinked = true;
                }

                // Проверка нажатия кнопки во второй половине секунды (пауза/выход)
                if (elapsed > 500 && digitalRead(_btn) == LOW) {
                    if (handlePauseAndExit(num, blinked)) {
                        return; // Прерываем таймер и возвращаемся в loop
                    }
                }
            }

            // Если фаза мигания активна, возвращаем число перед новой секундой
            if (blinked) {
                _disp.clearPrintR(num);
                _disp.update();
            }
        }

        // Таймер дошел до конца, переходим в режим ожидания сброса
        waitForReset();
    }

  private:
    DispBare<DIGITS, true, true> _disp; // Объект дисплея из GyverSegment (общий анод)
    uint8_t _btn;                       // Номер пина кнопки

    /**
     * Внутренний метод обработки паузы и аварийного выхода.
     * Возвращает true, если нужно полностью прервать выполнение таймера.
     */
    bool handlePauseAndExit(int currentNum, bool& blinked) {
        // Ожидаем отпускания кнопки (встали на паузу)
        while (digitalRead(_btn) == LOW) {
            _disp.tick();
        }
        
        // Во время паузы возвращаем текущее число на экран, чтобы оно горело стабильно
        if (blinked) {
            _disp.clearPrintR(currentNum);
            _disp.update();
        }

        // Ждем следующего нажатия кнопки (сигнал к сбросу)
        while (digitalRead(_btn) == HIGH) {
            _disp.tick();
        }
        // Ждем отпускания после нажатия на сброс
        while (digitalRead(_btn) == LOW) {
            _disp.tick();
        }

        // Анимация возврата к исходному состоянию
        delay(200);
        showZero();
        delay(500);
        return true; 
    }

    /**
     * Ожидание нажатия кнопки для сброса после того, как таймер успешно завершился.
     */
    void waitForReset() {
        while (digitalRead(_btn) == HIGH) {
            _disp.tick();
        }
        while (digitalRead(_btn) == LOW) {
            _disp.tick();
        }
        delay(200);
        showZero();
        delay(500);
    }
};