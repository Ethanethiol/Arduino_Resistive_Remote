#include <Arduino.h> //Подключаем ядро Arduino для PlatformIO. Если вы пользуетесь Arduino IDE, удалите эту строку
#define debugValue 1 // 1 = вывод в serial значения функции считывания кнопок. Не оставляйте значение пустым
#define debugKey 1   // 1 = вывод в serial кода считанной кнопки. Не оставляйте значение пустым

//===Объявляем константы и переменные===
const int NUM_READ=196;      //Количество опросов кнопки для фильтра шумов
const int useLevel=680;    //Уровень сигнала пульта, выше которого считаем все кнопки отпущеными
const int readDelay=0;    //задержка на устаканивание ШИМ при опросе кнопок
const int keyTolerance=7;  //Допустимое дрожание сигнала при опросе кнопок (принимаем событие нажатия кнопки если два последовательных осредненых считывания отличаются в пределах этой величины)
const int pushDelay=0;    //Таймаут подтверждения кода кнопки в миллисекундах. Код передается в программу после удержания кнопки в течение /pushDelay/. Можно использовать для обработки комбинаций кнопок (нужно успеть зажать комбинацию в течение pushDelay).
int prevButton=0;          //используем для однозначного определения кода выбранной кнопки в основном цикле
int mainLevel=1023;        //Уровень сигнала от пульта без нажатия кнопок (для вычислений) 1023 - максимальное значение для АЦП Arduino Nano



//===Назначаем пины Arduino===
int swPin = A7;                                //Пин подключения резистивного пульта руля
//---Пины для эмуляции кнопок штатного руля Meriva A {
int STDVOL_DN = 7;
int STDVOL_UP = 6;  
int STDSOURCE = 5;
int STDARR_RIGHT = 4;
int STDARR_LEFT = 3;
int STDRELEASE = 2;
//---}
//---Пины управления Bluetooth {
int BT_PLAY = 10;
int BT_RR = 11;
int BT_FF = 12;
//---}

//--- Функция измерения сопротивления (с фильтром шумов){
int KeyResistance() {
  long sum = 0;                       // локальная переменная sum
  for (int i = 0; i < NUM_READ; i++) { // согласно количеству усреднений
  // delay (1);                        // задержка в 1мс для снижения влияния помех от ШИМ подсветки
    sum += analogRead(swPin);         // суммируем измеренные значения
  }
  return ((float)sum / NUM_READ);
}
//---}

//--- Функция отсечения величины паразитного потенциала от параллельных цепей{
// Вызываем KeyResistance(), если значение соответствует отпущенным кнопкам (useLevel) - запоминаем в mainLevel.
// Если значение меньше порога для отпущенных кнопок, вычитаем значение из mainLevel, т.е. величина паразитного потенциала обнуляется. Правда, значение получается инвертированным. Ну и ладно...
int keyRAW () {
  int prevKey = 0; //Объявлем локальную переменную для сравнения последовательных считываний кнопки. 1023 - максимальное значение АЦП Arduino Nano
  int Key = KeyResistance();

do { //устраняем дрожание ШИМ от цепи подсветки кнопок. Читаем KeyResistance()дважды, если второе считывание отличается от первого более чем на keyTolerance, читаем снова и сравниваем с предпоследним. И так, пока не совпадет
  prevKey = Key;
  delay (readDelay);
  Key = KeyResistance();}
while (Key > prevKey+keyTolerance || Key < prevKey-keyTolerance);

    if (prevKey > useLevel) {
    mainLevel = prevKey;
    return (mainLevel+200); //это сделано для контроля состояния отпущенных кнопок при настройке значений. "+200" - чтобы вывести значение из диапазона значений для кнопок
  }
 return (mainLevel - prevKey);
}
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
    int Key = keyRAW();  //Читаем сопротивление пульта руля в переменную "Key"
    if (debugValue==1) Serial.println(Key); 
    //---Устанавливаем соответствие считанных сопротивлений кнопкам---
      if (Key >= 478 && Key <= 500) return (VOL_UP);
      if (Key >= 447 && Key <= 477) return (VOL_DN);
      if (Key >= 420 && Key <= 443) return (ARR_UP);
      if (Key >= 250 && Key <= 370) return (ARR_DN);
      if (Key >= 693 && Key <= 718) return (LW_UP);
      if (Key >= 650 && Key <= 683) return (LW_DN);
      if (Key >= 610 && Key <= 640) return (LW_PRESS);
      if (Key >= 520 && Key <= 570) return (LK_UP);
      if (Key >= 390 && Key <= 410) return (LK_DN);
      return (0);
}
//<== Конец Функции чтения кодов кнопок руля===




void setup() {
  
  if (debugValue==1 || debugKey==1 )
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
  //Пины управления Bluetooth прижимаем к земле. Если используем в схеме преобразователь логических уровней, он без этого будет слать на выход половинный уровень.
  pinMode (BT_PLAY, OUTPUT);
  digitalWrite (BT_PLAY, LOW);
  pinMode (BT_RR, OUTPUT);
  digitalWrite (BT_RR, LOW);
  pinMode (BT_FF, OUTPUT);
  digitalWrite (BT_FF, LOW);
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

switch (currButton) // Выбираем действие, в зависимости от значения currButton
{
case ARR_UP:
  digitalWrite (BT_FF, HIGH);
  if (debugKey == 1) Serial.println("ARR_UP");
break;
case ARR_DN:
  digitalWrite (BT_RR, HIGH);
  if (debugKey == 1) Serial.println("ARR_DN");
break;
case VOL_UP:
  pinMode(STDVOL_UP, OUTPUT);
  digitalWrite(STDVOL_UP, LOW);
  if (debugKey == 1) Serial.println("VOL_UP");
break;
case VOL_DN:
  pinMode(STDVOL_DN, OUTPUT);
  digitalWrite(STDVOL_DN, LOW);
  if (debugKey == 1) Serial.println("VOL_DN");
break;
case LW_UP:
  pinMode(STDARR_RIGHT, OUTPUT);
  digitalWrite(STDARR_RIGHT, LOW);
  if (debugKey == 1) Serial.println("LW_UP");
break;
case LW_DN:
  pinMode(STDARR_LEFT, OUTPUT);
  digitalWrite(STDARR_LEFT, LOW);
  if (debugKey == 1) Serial.println("LW_DN");
break;
case LW_PRESS:
  pinMode(STDRELEASE, OUTPUT);
  digitalWrite(STDRELEASE, LOW);
  if (debugKey == 1) Serial.println("LW_PRESS");
break;
case LK_UP:
  digitalWrite (BT_PLAY, HIGH);
  if (debugKey == 1) Serial.println("LK_UP");
break;
case LK_DN:
  pinMode(STDSOURCE, OUTPUT);
  digitalWrite(STDSOURCE, LOW);
  if (debugKey == 1) Serial.println("LK_DN");
break;
default:
  pinMode(STDVOL_DN, INPUT);
  pinMode(STDVOL_UP, INPUT);
  pinMode(STDSOURCE, INPUT);
  pinMode(STDARR_RIGHT, INPUT);
  pinMode(STDARR_LEFT, INPUT);
  pinMode(STDRELEASE, INPUT);
  digitalWrite (BT_PLAY, LOW);
  digitalWrite (BT_RR, LOW);
  digitalWrite (BT_FF, LOW);
  if (debugKey == 1) Serial.println("====");
  delay (1);
break;
}
}
}
 }
