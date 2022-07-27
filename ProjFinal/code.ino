#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <queue.h>
#include <task.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

/// Mutex Semaphore Handle que controla a Serial Port.
/// Apenas uma tarefa controla a serial de cada vez.
SemaphoreHandle_t xSerialSemaphore;
/// Estrutura utilizada para ler dados do sensor.
struct pinRead
{
    int pin;     /// Pino lido da placa.
    float value; /// Valor lido.
};

int flag; /// Controla o buffer para cálculo da Média.
int k;    /// Contador de preenchimento do buffer da Média.
int i;    /// Variável de controle do buzzer.

///  Vetor que guarda 10 dados lidos do sensor para ser
///  Calculada a média pela task TempMedia.
float bufferTemp[10];

const int pinBuzzer = 11; /// Porta digital na qual o buzzer está ligado

///  Handle da fila que a task AnalogRead envia dados lidos do sensor.
QueueHandle_t structQueue;

void TaskAnalogRead(void *pvParameters);
void TaskTempAtual(void *pvParameters);
void TaskTempMedia(void *pvParameters);
void TaskAtuador(void *pvParameters);

void setup() /// Função que executa quando liga a placa ou aperta o botão reset.
{

    lcd.init();      // inicializa LCD
    lcd.backlight(); // acende backlight
    lcd.clear();     // limpa a tela (caracteres)
    //
    lcd.setCursor(0, 0);      // endereça cursor
    lcd.print("BOH ALMOCAR"); // escreve
    lcd.setCursor(0, 1);
    lcd.print("AS 13H");
    lcd.setCursor(0, 2);
    lcd.print("MEU AMOIZINHO?");
    lcd.setCursor(0, 3);
    lcd.print(" TI AMU METE BALA <3");

    pinMode(2, OUTPUT);
    pinMode(3, OUTPUT);
    pinMode(4, OUTPUT);
    pinMode(5, OUTPUT);
    pinMode(6, OUTPUT);

    /// Inicia a comunicação serial a 9600 bits por segundo.
    Serial.begin(9600);
    while (!Serial)
    {
        ; /// Espera a porta serial conectar.
    }
    Serial.print("Iniciando rotina"); /// Confirma que a conexão foi estabelecida.(Debug)

    pinMode(pinBuzzer, OUTPUT);

    if (xSerialSemaphore == NULL)
    {                                               /// Checa se o semáforo da porta serial já não foi criado.
        xSerialSemaphore = xSemaphoreCreateMutex(); /// Cria a mutex que controla a porta serial.
        if ((xSerialSemaphore) != NULL)
            xSemaphoreGive((xSerialSemaphore)); /// Torna a porta serial disponível, "dando" o semáforo.
    }

    /// Cria a fila de dados do sensor.
    structQueue = xQueueCreate(10, sizeof(struct pinRead));

    if (structQueue != NULL)
    { /// Verifica se a fila foi criada.
        /// Cria tarefas que serão executadas independentemente.

        xTaskCreate(TaskTempAtual, "TempAtual", 128, NULL, 2, NULL); /// Cria a tarefa para consumir dados da fila.

        xTaskCreate(TaskTempMedia, "TempMedia", 128, NULL, 2, NULL); /// Cria a tarefa para cálculo da média.

        xTaskCreate(TaskAnalogRead, "AnalogRead", 128, NULL, 2, NULL); /// Cria a tarefa produtora de dados da fila.

        xTaskCreate(TaskAtuador, "BuzzerTone", 128, NULL, 2, NULL); /// Cria a tarefa produtora de dados da fila.
    }
    /// Agora, o escalonador de tarefas, que assume o controle do escalonamento de tarefas individuais, é iniciado automaticamente.*/
}

void loop()
{
    /// Vazio. Tudo é feito nas tarefas.
}

///*--------------------------------------------------*/
///*---------------------- Tasks ---------------------*/
///*--------------------------------------------------*/

void TaskAnalogRead(void *pvParameters __attribute__((unused))) /// Tarefa que lê dados do sensor.
{
    for (;;)
    {
        struct pinRead currentPinRead;
        currentPinRead.pin = 0;
        ///  Codificação dos valores lidos em tensão para temperatura.
        /// Fonte: https://portal.vidadesilicio.com.br/lm35-medindo-temperatura-com-arduino/
        currentPinRead.value = (float(analogRead(A0)) * 5 / (1023)) / 0.01;

        /// Posta um item na fila.
        /// https://www.freertos.org/a00117.html

        xQueueSend(structQueue, &currentPinRead, portMAX_DELAY);
        vTaskDelay(1); /// Um tick de atraso (15ms) entre as leituras para estabilidade.
    }
}

void TaskTempAtual(void *pvParameters __attribute__((unused))) /// Tarefa que consome dado do buffer se disponível;
{
    for (;;)
    {
        struct pinRead currentPinRead;

        /// Read an item from a queue.
        /// https://www.freertos.org/a00118.html
        if (xQueueReceive(structQueue, &currentPinRead, portMAX_DELAY) == pdPASS)
        {
            bufferTemp[k] = currentPinRead.value;
            if (k < 10)
            {             /// Verifica se ainda não foram armazenados 10 dados no buffer da média.
                i = k;    /// A variável de controle do buzzer recebe contador do buffer.
                flag = 0; /// Caso não, flag continua em 0.
                if (xSemaphoreTake(xSerialSemaphore, (TickType_t)5) == pdTRUE)
                {
                    /// Se o semáforo estiver disponível, a tarefa consegue o controle da porta serial.
                    Serial.print("Temp Atual: "); /// Comunica o valor lido da fila.
                    Serial.println(currentPinRead.value);
                    Serial.println(k);                /// Posição do buffer no momento.
                    xSemaphoreGive(xSerialSemaphore); /// Libera a porta serial.
                    k = k + 1;                        /// Incrementa a variável de controle do buffer.
                }
            }
            else
            {             /// Caso o contador atinja 10,
                i = 0;    /// reseta a variável de controle do buzzer para evitar leitura do buffer.
                flag = 1; /// Altera a flag e sinaliza que a média pode ser calculada.
            }
        }
    }
}

void TaskTempMedia(void *pvParameters __attribute__((unused))) /// Tarefa que consome o buffer para cálculo da média.
{
    for (;;)
    {
        float media;     /// Variável que guarda a média.
        float acumulado; /// Variável que guarda o acumulado dos dados do buffer.

        if (flag == 1)
        { /// Verifica se a flag foi alterada para 1.
            /// Executa laço para cálculo da média dos valores guardados no buffer.
            for (int j = 0; j < 10; j++)
            {
                acumulado = acumulado + bufferTemp[j];
            }
            media = acumulado / 10;

            flag = 0; /// Reseta a flag para confirmar que os dados do buffer foram consumidos e podem ser substituidos.
            k = 0;    /// Reseta variável de controle do buffer.

            if (xSemaphoreTake(xSerialSemaphore, (TickType_t)5) == pdTRUE)
            {
                /// Verifica se a porta serial está disponível.
                /// Caso obtenha o controle do semáforo,
                Serial.print("Media: "); /// comunica o valor da média pela porta serial.
                Serial.println(media);
                media = 0;                        /// Reseta a variável da média.
                acumulado = 0;                    /// Reseta variável do acumulado.
                xSemaphoreGive(xSerialSemaphore); /// Libera a porta serial.
            }
        }
        else
        {
            flag = 0;
            i = k;
        } /// Se ainda não foram feitas 10 leituras, a média não será calculada.
    }
}

void TaskAtuador(void *pvParameters __attribute__((unused))) /// Tarefa que consome o buffer para cálculo da média.
{
    /// Fonte:http://www.squids.com.br/arduino/index.php/projetos-arduino/projetos-squids/basico/137-projeto-36-controlando-frequencia-de-um-buzzer-com-potenciometro
    for (;;)
    {
        int frequency; /// Frequência tocada no buzzer.
        float atual;   /// Variável para guardar o valor consumido do buffer.

        if (i > 0)
        {
            atual = bufferTemp[i]; /// Consome dado do buffer.
            if (atual > 5 && atual <= 20)
            { /// Caso a temperatura atinja valor superior a 29, codifica os valores de temperatura
                // frequency = map(atual, 30, 80, 0, 2500); /// entre 30 e 80 para valores de frequencia entre 0 e 2500.
                //        tone(pinBuzzer, frequency);  /// Buzzer emite som para cada leitura acima de 29.

                //        lcd.clear();
                //        lcd.setCursor(0, 0);
                //        lcd.print(atual);    // Exibe o valor de temperatura no display.
                //        lcd.print(" C");         // Escreve “C” para dizer que a escala é Celsius.

                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Temp Ambiente abaixo"); // Exibe o valor de temperatura no display.
                lcd.setCursor(0, 1);
                lcd.print("do Ideal"); // Escreve “C” para dizer que a escala é Celsius.
                lcd.setCursor(0, 2);
                lcd.print("Aumentando a Temp");
                lcd.setCursor(0, 3);
                lcd.print("do Ar-Condicionado");
                delay(300);

                digitalWrite(3, LOW);
                digitalWrite(2, HIGH);
            }
            else if (atual > 20 && atual <= 23)
            {
                //        frequency = map(atual, 30, 80, 0, 2500); /// entre 30 e 80 para valores de frequencia entre 0 e 2500.
                //        tone(pinBuzzer, frequency);  /// Buzzer emite som para cada leitura acima de 29.
                //        lcd.clear();
                //        lcd.setCursor(0, 0);           // Move o cursor do display para a segunda linha.
                //        lcd.print(atual);    // Exibe o valor de temperatura no display.
                //        lcd.print(" C");         // Escreve “C” para dizer que a escala é Celsius.

                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Temp Ambiente Ideal"); // Exibe o valor de temperatura no display.
                lcd.setCursor(0, 1);
                lcd.print("Stand-By Ativado"); // Escreve “C” para dizer que a escala é Celsius.
                delay(300);

                digitalWrite(2, LOW);
                digitalWrite(4, LOW);
                digitalWrite(3, HIGH);
            }
            else if (atual > 23 && atual <= 33)
            {
                //        frequency = map(atual, 30, 80, 0, 2500); /// entre 30 e 80 para valores de frequencia entre 0 e 2500.
                //        tone(pinBuzzer, frequency);  /// Buzzer emite som para cada leitura acima de 29.
                //        lcd.clear();
                //        lcd.setCursor(0, 0);           // Move o cursor do display para a segunda linha.
                //        lcd.print(atual);    // Exibe o valor de temperatura no display.
                //        lcd.print(" C");         // Escreve “C” para dizer que a escala é Celsius.

                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Temp Ambiente acima"); // Exibe o valor de temperatura no display.
                lcd.setCursor(0, 1);
                lcd.print("do Ideal"); // Escreve “C” para dizer que a escala é Celsius.
                lcd.setCursor(0, 2);
                lcd.print("Diminuindo a Temp");
                lcd.setCursor(0, 3);
                lcd.print("do Ar-Condicionado");
                delay(300);

                digitalWrite(3, LOW);
                digitalWrite(5, LOW);
                digitalWrite(4, HIGH);
            }
            else if (atual > 33 && atual <= 42)
            {
                //        frequency = map(atual, 30, 80, 0, 2500); /// entre 30 e 80 para valores de frequencia entre 0 e 2500.
                //        tone(pinBuzzer, frequency);  /// Buzzer emite som para cada leitura acima de 29.
                //        lcd.clear();
                //        lcd.setCursor(0, 0);           // Move o cursor do display para a segunda linha.
                //        lcd.print(atual);    // Exibe o valor de temperatura no display.
                //        lcd.print(" C");         // Escreve “C” para dizer que a escala é Celsius.

                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Temp Ambiente muito"); // Exibe o valor de temperatura no display.
                lcd.setCursor(0, 1);
                lcd.print("acima da Ideal"); // Escreve “C” para dizer que a escala é Celsius.
                lcd.setCursor(0, 2);
                lcd.print("Diminuindo a Temp");
                lcd.setCursor(0, 3);
                lcd.print("do Ar-Condicionado");
                delay(300);

                digitalWrite(6, LOW);
                digitalWrite(4, LOW);
                digitalWrite(5, HIGH);
            }
            else if (atual > 42 && atual < 100)
            {
                frequency = map(atual, 30, 80, 0, 2500); /// entre 30 e 80 para valores de frequencia entre 0 e 2500.
                tone(pinBuzzer, frequency);              /// Buzzer emite som para cada leitura acima de 29.

                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("PERIGO!!!"); // Exibe o valor de temperatura no display.
                lcd.setCursor(0, 2);
                lcd.print("RISCO DE INCENDIO"); // Escreve “C” para dizer que a escala é Celsius.
                delay(300);

                digitalWrite(5, LOW);
                digitalWrite(6, HIGH);

                //        lcd.clear();
                //        lcd.setCursor(0, 0);           // Move o cursor do display para a segunda linha.
                //        lcd.print(atual);    // Exibe o valor de temperatura no display.
                //        lcd.print(" C");         // Escreve “C” para dizer que a escala é Celsius.
            } /// Se não for detectada temperatura superior a 29, o som do buzzer apenas será alterado na chamada de noTone().
        }
        else
        {                      /// Se não há dados no buffer (k=0), buzzer não consome dados.
            i = 0;             /// Reseta variável de controle do buzzer.
            noTone(pinBuzzer); /// Silencia o buzzer até a próxima chamada de tone().
        }
    }
}