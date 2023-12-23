#include <Arduino.h> //Подключаем ядро Arduino для PlatformIO. Если вы пользуетесь Arduino IDE, удалите эту строку

//===Объявляем переменные===
int pushDelay=10;                              //Таймаут подтверждения кода кнопки в миллисекундах. Код передается в программу после удержания кнопки в течение /pushDelay/. Можно использовать для обработки комбинаций кнопок.
int prevButton=0;                              //используем для однозначного определения кода выбранной кнопки в основном цикле

//===Назначаем пины Arduino===
int swPin = A7;                                //Пин подключения пульта руля
//---Пины для эмуляции кнопок штатного руля Meriva A {
int STDVOL_DN = 7;
int STDVOL_UP = 6; 
int STDSOURCE = 5;
int STDARR_RIGHT = 4;
int STDARR_LEFT = 3;
int STDRELEASE = 2;
//---}


//===Команды и коды кнопок руля Astra H (TAKATA-Petri)===
const int VOL_UP = 100;                         //Правое колесико вверх
const int VOL_DN = 101;                         //Правое колесико вниз
const int ARR_UP = 102;                         //Стрелка вверх
const int ARR_DN = 103;                         //Стрелка вниз
const int LW_UP = 104;                          //Левое колесико вверх
const int LW_DN = 105;                          //Левое колесико вниз
const int LW_PRESS = 106;                       //Левое колесико нажатие
const int LK_UP = 107;                          //Левый пульт верхняя кнопка
const int LK_DN = 108;                          //Левый пульт нижняя кнопка

//===Функция чтения кодов кнпок руля==>
int getKey() {

//Читаем сопротивление пульта руля в переменную "Key"
int Key = analogRead(swPin);
//Вывод значения в Serial для первичной настройки значений кнопок. При компиляции финальной прошивки закомментить. 
//Serial.println(Key);
//---Устанавливаем соответствие считанных сопротивлений кнопкам---
if (Key >= 220 && Key <= 235) return (VOL_UP);
if (Key >= 240 && Key <= 255) return (VOL_DN);
if (Key >= 275 && Key <= 290) return (ARR_UP);
if (Key >= 350 && Key <= 365) return (ARR_DN);
if (Key >= 15 && Key <= 35) return (LW_UP);
if (Key >= 45 && Key <= 60) return (LW_DN);
if (Key >= 90 && Key <= 105) return (LW_PRESS);
if (Key >= 165 && Key <= 185) return (LK_UP);
if (Key >= 310 && Key <= 330) return (LK_DN);
return (0);
}
//<== Конец Функции чтения кодов кнпок руля===




void setup() {
  Serial.begin(9600); //Включаем интерфейс последовательного порта для вывода контрольной информации в консоль на компьютер
  
  analogReference(INTERNAL);//Опорное напряжение для диапазона сопротивлений до 2,2кОм = 1.1В "INTERNAL", свыше 2,2кОм = 5В "DEFAULT"
  
  //Объявляем пин подключения руля входом. (Для порядку)
  pinMode(swPin, INPUT);
  
  //Устанавливаем пины эмуляции кнопок в режим INPUT для установки между ними и землей максимально возможного сопротивления, т.е. "отрываем" их от земли.
  pinMode(STDVOL_DN, INPUT);
  pinMode(STDVOL_UP, INPUT);
  pinMode(STDSOURCE, INPUT);
  pinMode(STDARR_RIGHT, INPUT);
  pinMode(STDARR_LEFT, INPUT);
  pinMode(STDRELEASE, INPUT);
}

// the loop routine runs over and over again forever:
void loop() {
  int currButton = getKey(); // заносим в переменную currButton код нажатой кнопки
    if (currButton != prevButton) // если значение поменялось с прошлого раза
{
    delay(pushDelay);
    currButton = getKey(); // ждем /pushDelay/ и читаем еще раз, чтобы убедиться в выборе события

    if (currButton != prevButton) // если код кнопки точно поменялся с прошлого раза
{
    prevButton = currButton; // сохраняем новое значение в переменную prevButton


switch (currButton)
{
case VOL_UP:
  pinMode(STDVOL_UP, OUTPUT);
  digitalWrite(STDVOL_UP, LOW);
  Serial.println("VOL_UP");
break;
case VOL_DN:
  pinMode(STDVOL_DN, OUTPUT);
  digitalWrite(STDVOL_DN, LOW);
  Serial.println("VOL_DN");
break;
case ARR_UP:
  Serial.println("ARR_UP");
break;
case ARR_DN:
  Serial.println("ARR_DN");
break;
case LW_UP:
  pinMode(STDARR_RIGHT, OUTPUT);
  digitalWrite(STDARR_RIGHT, LOW);
  Serial.println("LW_UP");
break;
case LW_DN:
  pinMode(STDARR_LEFT, OUTPUT);
  digitalWrite(STDARR_LEFT, LOW);
  Serial.println("LW_DN");
break;
case LW_PRESS:
  pinMode(STDRELEASE, OUTPUT);
  digitalWrite(STDRELEASE, LOW);
  Serial.println("LW_PRESS");
break;
case LK_UP:
  pinMode(STDSOURCE, OUTPUT);
  digitalWrite(STDSOURCE, LOW);
  Serial.println("LK_UP");
break;
case LK_DN:
  Serial.println("LK_DN");
break;
default:
  Serial.println("====");
  pinMode(STDVOL_DN, INPUT);
  pinMode(STDVOL_UP, INPUT);
  pinMode(STDSOURCE, INPUT);
  pinMode(STDARR_RIGHT, INPUT);
  pinMode(STDARR_LEFT, INPUT);
  pinMode(STDRELEASE, INPUT);
  break;
}
}
}
  delay(1);  // delay in between reads for stability
 }
