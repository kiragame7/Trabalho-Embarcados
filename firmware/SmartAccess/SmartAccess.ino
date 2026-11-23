/*
 * ============================================================================
 *  SmartAccess - Sistema inteligente de controle de acesso com RFID
 *  Disciplina: Sistemas Embarcados - Projeto Final
 *  Plataforma: Arduino Uno
 * ============================================================================
 *
 *  Funcionamento resumido:
 *   - Cartao autorizado  -> LED verde, bip, servo abre a trava por 4 s.
 *   - Cartao desconhecido -> LED vermelho, bip grave, porta continua fechada.
 *   - 3 tentativas invalidas seguidas -> bloqueio de 30 s com contagem no LCD.
 *   - Cartao ADMIN -> menu de manutencao (CADASTRAR / EXCLUIR).
 *       * botao MODO alterna entre cadastrar e excluir;
 *       * passar o cartao ADMIN de novo sai do menu;
 *       * o menu expira apos 15 s sem acao.
 *   - Cartoes cadastrados ficam salvos na EEPROM (nao se perdem ao desligar).
 *   - Segurar o botao MODO ao ligar apaga todos os cadastros.
 *
 *  Comunicacoes usadas:
 *   - SPI : leitor RFID RC522
 *   - I2C : display LCD 16x2
 *
 *  Comandos no Monitor Serial (9600 baud):
 *   - LISTA  -> mostra os cartoes cadastrados
 *   - AJUDA  -> mostra os comandos
 *   - Em MODO_SIMULACAO: digitar um UID (ex.: "12 AB 34 CD") simula um cartao.
 *
 *  ATENCAO: prototipo didatico. A identificacao por UID pode ser clonada e
 *  nao deve ser usada como fechadura de seguranca real.
 * ============================================================================
 */

// ============================================================================
//  MODO DE COMPILACAO
//   1 = Simulacao no Wokwi (o Wokwi nao tem RC522: os cartoes sao simulados
//       por botoes e pelo Monitor Serial)
//   0 = Montagem fisica com o leitor RC522
// ============================================================================
#define MODO_SIMULACAO 1

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <EEPROM.h>

#if !MODO_SIMULACAO
#include <SPI.h>
#include <MFRC522.h>
#endif

// ============================================================================
//  CONFIGURACAO - PINOS
// ============================================================================
const byte PINO_RFID_SS      = 10;  // SPI: SDA/SS do RC522
const byte PINO_RFID_RST     = 9;   // RST do RC522
const byte PINO_SERVO        = 3;   // Sinal do servo (trava)
const byte PINO_BUZZER       = 4;   // Buzzer
const byte PINO_LED_VERDE    = 5;   // LED de acesso liberado
const byte PINO_LED_VERMELHO = 6;   // LED de acesso negado
const byte PINO_BOTAO_MODO   = 7;   // Botao: alterna CADASTRAR/EXCLUIR

#if MODO_SIMULACAO
// Botoes que fazem o papel de "aproximar um cartao" no Wokwi
const byte PINO_SIM_ADMIN   = A0;  // Cartao ADMIN
const byte PINO_SIM_CARTAO1 = A1;  // Cartao 1 (ja vem cadastrado)
const byte PINO_SIM_CARTAO2 = A2;  // Cartao 2 (comeca desconhecido)
#endif

// LED RGB de 4 perninhas: perna comprida no GND -> false (catodo comum)
//                        perna comprida no 5V  -> true  (anodo comum)
// LEDs separados (verde e vermelho comuns): deixar false.
const bool LED_ANODO_COMUM = false;

const byte ENDERECO_LCD = 0x27;     // Trocar para 0x3F se o LCD nao mostrar nada

// ============================================================================
//  CONFIGURACAO - TEMPOS (ms) E PARAMETROS
// ============================================================================
const unsigned long TEMPO_PORTA_ABERTA = 4000UL;
const unsigned long TEMPO_MSG_NEGADO   = 1500UL;
const unsigned long TEMPO_BLOQUEIO     = 30000UL;
const unsigned long TEMPO_ADMIN        = 15000UL;
const unsigned long TEMPO_MSG_ADMIN    = 1500UL;
const unsigned long TEMPO_DEBOUNCE     = 50UL;
const unsigned long TEMPO_PRESSAO_LONGA = 3000UL;  // segurar MODO 3 s = entra/sai do admin

const byte MAX_TENTATIVAS = 3;

const byte ANGULO_FECHADO = 0;      // Ajustar conforme a maquete
const byte ANGULO_ABERTO  = 90;

// ============================================================================
//  CONFIGURACAO - CARTOES
// ============================================================================
// SUBSTITUIR pelo UID real do cartao administrador (aparece no Monitor Serial).
const byte UID_ADMIN[4] = {0xDE, 0xAD, 0xBE, 0xEF};

#if MODO_SIMULACAO
const byte UID_SIM_CARTAO1[4] = {0x11, 0x22, 0x33, 0x44};
const byte UID_SIM_CARTAO2[4] = {0x55, 0x66, 0x77, 0x88};
#endif

// ============================================================================
//  CONFIGURACAO - EEPROM
//   Endereco 0 : assinatura (indica que a memoria ja foi formatada)
//   Endereco 1 : quantidade de cartoes cadastrados
//   Endereco 2+: UIDs, 4 bytes cada
// ============================================================================
const byte MAX_CARTOES       = 15;
const byte EEPROM_ASSINATURA = 0xA8;
const int  END_ASSINATURA    = 0;
const int  END_QUANTIDADE    = 1;
const int  END_CARTOES       = 2;

// ============================================================================
//  TIPOS
// ============================================================================
// Estados da maquina de estados principal
enum Estado {
  AGUARDANDO,      // Esperando um cartao
  PORTA_ABERTA,    // Acesso liberado, trava aberta
  ACESSO_NEGADO,   // Mensagem de negacao na tela
  BLOQUEADO,       // Bloqueio temporario apos tentativas invalidas
  MODO_ADMIN       // Menu de cadastro/exclusao
};

// Resultado de uma operacao do menu administrador
enum ResultadoAdmin {
  CADASTRADO,
  JA_CADASTRADO,
  MEMORIA_CHEIA,
  EXCLUIDO,
  NAO_CADASTRADO,
  ADMIN_PROTEGIDO
};

// Botao com debounce feito por millis()
struct Botao {
  byte pino;
  bool leituraAnterior;
  bool estadoEstavel;
  unsigned long ultimaMudanca;
};

// ============================================================================
//  OBJETOS E VARIAVEIS GLOBAIS
// ============================================================================
LiquidCrystal_I2C lcd(ENDERECO_LCD, 16, 2);
Servo servoTrava;

#if !MODO_SIMULACAO
MFRC522 leitorRfid(PINO_RFID_SS, PINO_RFID_RST);
#endif

Botao botaoModo = {PINO_BOTAO_MODO, HIGH, HIGH, 0};
#if MODO_SIMULACAO
Botao botaoSimAdmin   = {PINO_SIM_ADMIN,   HIGH, HIGH, 0};
Botao botaoSimCartao1 = {PINO_SIM_CARTAO1, HIGH, HIGH, 0};
Botao botaoSimCartao2 = {PINO_SIM_CARTAO2, HIGH, HIGH, 0};
#endif

Estado estadoAtual = AGUARDANDO;
unsigned long estadoDesde = 0;       // Momento em que entrou no estado atual
byte tentativasInvalidas = 0;

bool adminModoExcluir = false;       // false = CADASTRAR, true = EXCLUIR
unsigned long adminUltimaAcao = 0;
bool adminMensagemAtiva = false;     // Resultado de uma operacao sendo exibido
unsigned long adminMensagemDesde = 0;

unsigned int ultimoSegundoMostrado = 0;
bool pressaoLongaUsada = false;      // evita repetir a acao enquanto o botao segue apertado

char bufferSerial[24];
byte tamanhoBufferSerial = 0;

// ============================================================================
//  DISPLAY LCD
// ============================================================================
// Escreve uma linha inteira (completa com espacos para apagar restos)
void escreverLinha(byte linha, const char* texto) {
  lcd.setCursor(0, linha);
  byte n = 0;
  while (texto[n] != '\0' && n < 16) {
    lcd.print(texto[n]);
    n++;
  }
  while (n < 16) {
    lcd.print(' ');
    n++;
  }
}

void mostrarTela(const char* linha1, const char* linha2) {
  escreverLinha(0, linha1);
  escreverLinha(1, linha2);
}

// ============================================================================
//  SINAIS (LEDs e BUZZER)
// ============================================================================
// Liga/desliga um LED levando em conta se o LED RGB e anodo comum (logica invertida)
void ligarLed(byte pino, bool ligado) {
  if (LED_ANODO_COMUM) ligado = !ligado;
  digitalWrite(pino, ligado ? HIGH : LOW);
}

void apagarLeds() {
  ligarLed(PINO_LED_VERDE, false);
  ligarLed(PINO_LED_VERMELHO, false);
}

// tone() com duracao nao bloqueia o programa
void bip(unsigned int frequencia, unsigned long duracao) {
  tone(PINO_BUZZER, frequencia, duracao);
}

// ============================================================================
//  UTILITARIOS DE UID
// ============================================================================
bool uidsIguais(const byte* a, const byte* b) {
  for (byte i = 0; i < 4; i++) {
    if (a[i] != b[i]) return false;
  }
  return true;
}

void copiarUid(byte* destino, const byte* origem) {
  for (byte i = 0; i < 4; i++) destino[i] = origem[i];
}

void imprimirUid(const byte* uid) {
  for (byte i = 0; i < 4; i++) {
    if (uid[i] < 0x10) Serial.print('0');
    Serial.print(uid[i], HEX);
    if (i < 3) Serial.print(' ');
  }
}

// ============================================================================
//  EEPROM - CADASTRO DE CARTOES
// ============================================================================
byte quantidadeCartoes() {
  byte qtd = EEPROM.read(END_QUANTIDADE);
  return (qtd <= MAX_CARTOES) ? qtd : 0;
}

void lerCartaoSalvo(byte indice, byte* uid) {
  int base = END_CARTOES + (int)indice * 4;
  for (byte i = 0; i < 4; i++) uid[i] = EEPROM.read(base + i);
}

void gravarCartao(byte indice, const byte* uid) {
  int base = END_CARTOES + (int)indice * 4;
  for (byte i = 0; i < 4; i++) EEPROM.update(base + i, uid[i]);
}

// Retorna a posicao do cartao na memoria, ou -1 se nao estiver cadastrado
int buscarCartao(const byte* uid) {
  byte salvo[4];
  byte qtd = quantidadeCartoes();
  for (byte i = 0; i < qtd; i++) {
    lerCartaoSalvo(i, salvo);
    if (uidsIguais(uid, salvo)) return i;
  }
  return -1;
}

ResultadoAdmin cadastrarCartao(const byte* uid) {
  if (uidsIguais(uid, UID_ADMIN)) return ADMIN_PROTEGIDO;
  if (buscarCartao(uid) >= 0) return JA_CADASTRADO;
  byte qtd = quantidadeCartoes();
  if (qtd >= MAX_CARTOES) return MEMORIA_CHEIA;
  gravarCartao(qtd, uid);
  EEPROM.update(END_QUANTIDADE, qtd + 1);
  return CADASTRADO;
}

// Exclui movendo o ultimo cartao da lista para a posicao removida
ResultadoAdmin excluirCartao(const byte* uid) {
  if (uidsIguais(uid, UID_ADMIN)) return ADMIN_PROTEGIDO;
  int indice = buscarCartao(uid);
  if (indice < 0) return NAO_CADASTRADO;
  byte qtd = quantidadeCartoes();
  byte ultimo[4];
  lerCartaoSalvo(qtd - 1, ultimo);
  gravarCartao(indice, ultimo);
  EEPROM.update(END_QUANTIDADE, qtd - 1);
  return EXCLUIDO;
}

void apagarMemoria() {
  EEPROM.update(END_ASSINATURA, 0x00);
  EEPROM.update(END_QUANTIDADE, 0);
}

void iniciarMemoria() {
  if (EEPROM.read(END_ASSINATURA) != EEPROM_ASSINATURA ||
      EEPROM.read(END_QUANTIDADE) > MAX_CARTOES) {
    Serial.println(F("EEPROM sem dados validos: formatando."));
    EEPROM.update(END_ASSINATURA, EEPROM_ASSINATURA);
    EEPROM.update(END_QUANTIDADE, 0);
#if MODO_SIMULACAO
    // Na simulacao, o Cartao 1 ja comeca autorizado para facilitar o teste
    cadastrarCartao(UID_SIM_CARTAO1);
#endif
  }
}

void listarCartoes() {
  byte qtd = quantidadeCartoes();
  Serial.print(F("Cartoes cadastrados: "));
  Serial.print(qtd);
  Serial.print('/');
  Serial.println(MAX_CARTOES);
  byte uid[4];
  for (byte i = 0; i < qtd; i++) {
    lerCartaoSalvo(i, uid);
    Serial.print(F("  ["));
    Serial.print(i + 1);
    Serial.print(F("] "));
    imprimirUid(uid);
    Serial.println();
  }
  Serial.print(F("Admin: "));
  imprimirUid(UID_ADMIN);
  Serial.println(F(" (fixo no codigo)"));
}

// ============================================================================
//  BOTOES (debounce sem delay)
// ============================================================================
void iniciarBotao(Botao& b) {
  pinMode(b.pino, INPUT_PULLUP);
}

// Retorna true apenas uma vez a cada aperto (borda de descida estavel)
bool foiPressionado(Botao& b) {
  bool leitura = digitalRead(b.pino);
  unsigned long agora = millis();
  if (leitura != b.leituraAnterior) {
    b.ultimaMudanca = agora;
    b.leituraAnterior = leitura;
  }
  if (agora - b.ultimaMudanca >= TEMPO_DEBOUNCE && leitura != b.estadoEstavel) {
    b.estadoEstavel = leitura;
    return leitura == LOW;   // INPUT_PULLUP: apertado = LOW
  }
  return false;
}

// ============================================================================
//  MONITOR SERIAL
// ============================================================================
void imprimirAjuda() {
#if MODO_SIMULACAO
  Serial.println(F("------------------------------------------------------------"));
  Serial.println(F("COMO TESTAR: clique nos BOTOES coloridos do circuito"));
  Serial.println(F("  VERDE   = encostar Cartao 1 (autorizado)   -> porta abre"));
  Serial.println(F("  CINZA   = encostar Cartao 2 (desconhecido) -> acesso negado"));
  Serial.println(F("  AZUL    = encostar cartao ADMIN            -> menu cadastro"));
  Serial.println(F("  AMARELO = no menu admin, troca CADASTRAR/EXCLUIR"));
  Serial.println(F("            segurar 3 s = entra/sai do menu admin"));
  Serial.println(F("Opcional aqui no Serial: LISTA (ver cadastrados) ou um UID"));
  Serial.println(F("completo de uma vez, ex.: 55 66 77 88"));
  Serial.println(F("------------------------------------------------------------"));
#else
  Serial.println(F("Comandos: LISTA (ver cadastrados), AJUDA"));
  Serial.println(F("Segurar o botao MODO 3 s = entra/sai do menu admin"));
#endif
}

// Converte texto como "12 AB 34 CD" em 4 bytes. Retorna true se valido.
bool converterUid(const char* texto, byte* uid) {
  byte digitos = 0;
  for (const char* p = texto; *p != '\0'; p++) {
    char c = *p;
    byte valor;
    if (c >= '0' && c <= '9')      valor = c - '0';
    else if (c >= 'A' && c <= 'F') valor = c - 'A' + 10;
    else if (c >= 'a' && c <= 'f') valor = c - 'a' + 10;
    else if (c == ' ' || c == ':' || c == '-') continue;
    else return false;

    if (digitos >= 8) return false;
    if (digitos % 2 == 0) uid[digitos / 2] = valor << 4;
    else                  uid[digitos / 2] |= valor;
    digitos++;
  }
  return digitos == 8;
}

// Le o Serial sem bloquear. Retorna 4 se uma linha formou um UID (simulacao).
byte lerSerial(byte* uid) {
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c != '\n' && c != '\r') {
      if (tamanhoBufferSerial < sizeof(bufferSerial) - 1) {
        bufferSerial[tamanhoBufferSerial++] = c;
      }
      continue;
    }
    if (tamanhoBufferSerial == 0) continue;   // linha vazia

    bufferSerial[tamanhoBufferSerial] = '\0';
    tamanhoBufferSerial = 0;
    for (byte i = 0; bufferSerial[i] != '\0'; i++) {
      bufferSerial[i] = toupper(bufferSerial[i]);
    }

    if (strcmp(bufferSerial, "LISTA") == 0) {
      listarCartoes();
    } else if (strcmp(bufferSerial, "AJUDA") == 0) {
      imprimirAjuda();
    } else {
#if MODO_SIMULACAO
      if (converterUid(bufferSerial, uid)) return 4;
#else
      (void)uid;   // no modo fisico, UID so vem do leitor RFID
#endif
      Serial.print(F("Nao entendi: \""));
      Serial.print(bufferSerial);
      Serial.println(F("\""));
#if MODO_SIMULACAO
      Serial.println(F("  Dica: use os BOTOES coloridos do circuito para simular cartoes."));
      Serial.println(F("  Para digitar um cartao, escreva os 4 pares DE UMA VEZ, ex.: 55 66 77 88"));
#endif
      Serial.println(F("  Comandos: LISTA (ver cadastrados), AJUDA"));
    }
  }
  return 0;
}

// ============================================================================
//  LEITURA DE CARTAO
//   Retorna o tamanho do UID lido (0 = nenhum cartao).
//   Somente UIDs de 4 bytes sao suportados nesta versao.
// ============================================================================
byte lerCartao(byte* uid) {
#if MODO_SIMULACAO
  if (foiPressionado(botaoSimAdmin))   { copiarUid(uid, UID_ADMIN);       return 4; }
  if (foiPressionado(botaoSimCartao1)) { copiarUid(uid, UID_SIM_CARTAO1); return 4; }
  if (foiPressionado(botaoSimCartao2)) { copiarUid(uid, UID_SIM_CARTAO2); return 4; }
#else
  if (leitorRfid.PICC_IsNewCardPresent() && leitorRfid.PICC_ReadCardSerial()) {
    byte tamanho = leitorRfid.uid.size;
    for (byte i = 0; i < 4 && i < tamanho; i++) uid[i] = leitorRfid.uid.uidByte[i];
    leitorRfid.PICC_HaltA();
    leitorRfid.PCD_StopCrypto1();
    return tamanho;
  }
#endif
  return lerSerial(uid);
}

// ============================================================================
//  TRANSICOES DE ESTADO
// ============================================================================
void mudarEstado(Estado novo) {
  estadoAtual = novo;
  estadoDesde = millis();
}

void entrarAguardando() {
  mudarEstado(AGUARDANDO);
  servoTrava.write(ANGULO_FECHADO);
  apagarLeds();
  mostrarTela("  SMARTACCESS", "Aproxime cartao");
}

void entrarPortaAberta() {
  mudarEstado(PORTA_ABERTA);
  tentativasInvalidas = 0;
  servoTrava.write(ANGULO_ABERTO);
  ligarLed(PINO_LED_VERMELHO, false);
  ligarLed(PINO_LED_VERDE, true);
  bip(2000, 150);
  mostrarTela("ACESSO LIBERADO", "Porta aberta");
  Serial.println(F("-> ACESSO LIBERADO"));
}

void entrarAcessoNegado(const char* linha2) {
  mudarEstado(ACESSO_NEGADO);
  ligarLed(PINO_LED_VERDE, false);
  ligarLed(PINO_LED_VERMELHO, true);
  bip(300, 500);
  mostrarTela("ACESSO NEGADO", linha2);
  Serial.println(F("-> ACESSO NEGADO"));
}

void entrarBloqueio() {
  mudarEstado(BLOQUEADO);
  tentativasInvalidas = 0;
  ultimoSegundoMostrado = 0;   // forca atualizar a contagem na tela
  ligarLed(PINO_LED_VERDE, false);
  bip(200, 1500);
  mostrarTela("  BLOQUEADO!", "");
  Serial.println(F("-> SISTEMA BLOQUEADO por excesso de tentativas"));
}

void mostrarMenuAdmin() {
  char linha2[17];
  snprintf(linha2, sizeof(linha2), "Cartoes: %02u/%02u",
           (unsigned int)quantidadeCartoes(), (unsigned int)MAX_CARTOES);
  mostrarTela(adminModoExcluir ? "ADMIN: EXCLUIR" : "ADMIN: CADASTRAR", linha2);
}

void entrarAdmin() {
  mudarEstado(MODO_ADMIN);
  adminModoExcluir = false;
  adminMensagemAtiva = false;
  adminUltimaAcao = millis();
  apagarLeds();
  bip(1200, 100);
  mostrarMenuAdmin();
  Serial.println(F("-> MODO ADMIN (botao MODO alterna cadastrar/excluir)"));
}

// ============================================================================
//  REGRAS DE NEGOCIO
// ============================================================================
void registrarTentativaInvalida() {
  tentativasInvalidas++;
  Serial.print(F("Tentativa invalida "));
  Serial.print(tentativasInvalidas);
  Serial.print(F(" de "));
  Serial.println(MAX_TENTATIVAS);

  if (tentativasInvalidas >= MAX_TENTATIVAS) {
    entrarBloqueio();
  } else {
    char linha2[17];
    snprintf(linha2, sizeof(linha2), "Tentativa %u/%u",
             (unsigned int)tentativasInvalidas, (unsigned int)MAX_TENTATIVAS);
    entrarAcessoNegado(linha2);
  }
}

void executarAcaoAdmin(const byte* uid) {
  ResultadoAdmin resultado = adminModoExcluir ? excluirCartao(uid) : cadastrarCartao(uid);
  const char* texto;
  bool sucesso = false;

  switch (resultado) {
    case CADASTRADO:      texto = "CADASTRADO!";     sucesso = true; break;
    case EXCLUIDO:        texto = "EXCLUIDO!";       sucesso = true; break;
    case JA_CADASTRADO:   texto = "JA CADASTRADO";   break;
    case MEMORIA_CHEIA:   texto = "MEMORIA CHEIA";   break;
    case NAO_CADASTRADO:  texto = "NAO CADASTRADO";  break;
    default:              texto = "ADMIN PROTEGIDO"; break;
  }

  char linha2[17];
  snprintf(linha2, sizeof(linha2), "Total: %02u/%02u",
           (unsigned int)quantidadeCartoes(), (unsigned int)MAX_CARTOES);
  mostrarTela(texto, linha2);
  bip(sucesso ? 2000 : 400, sucesso ? 150 : 300);

  Serial.print(F("Admin: "));
  Serial.println(texto);

  adminUltimaAcao = millis();
  adminMensagemAtiva = true;
  adminMensagemDesde = millis();
}

void tratarCartao(const byte* uid, byte tamanho) {
  if (tamanho != 4) {
    Serial.print(F("Cartao com UID de "));
    Serial.print(tamanho);
    Serial.println(F(" bytes: nao suportado (use UID de 4 bytes)."));
    if (estadoAtual == AGUARDANDO) {
      entrarAcessoNegado("UID INCOMPATIVEL");
    } else if (estadoAtual == MODO_ADMIN) {
      adminUltimaAcao = millis();
      mostrarTela("UID INCOMPATIVEL", "Use UID 4 bytes");
      adminMensagemAtiva = true;
      adminMensagemDesde = millis();
    }
    return;
  }

  Serial.print(F("Cartao lido: "));
  imprimirUid(uid);
  Serial.println();

  bool ehAdmin = uidsIguais(uid, UID_ADMIN);

  switch (estadoAtual) {
    case AGUARDANDO:
      if (ehAdmin)                     entrarAdmin();
      else if (buscarCartao(uid) >= 0) entrarPortaAberta();
      else                             registrarTentativaInvalida();
      break;

    case MODO_ADMIN:
      if (ehAdmin) {
        Serial.println(F("Saindo do modo admin."));
        entrarAguardando();
      } else {
        executarAcaoAdmin(uid);
      }
      break;

    case BLOQUEADO:
      // Nem o admin contorna o bloqueio: apenas avisa
      bip(200, 300);
      Serial.println(F("Ignorado: sistema bloqueado."));
      break;

    default:
      // PORTA_ABERTA ou ACESSO_NEGADO: ignora leituras ate voltar a aguardar
      Serial.println(F("Ignorado: aguarde a tela inicial."));
      break;
  }
}

void tratarBotaoModo() {
  if (estadoAtual != MODO_ADMIN) return;
  adminModoExcluir = !adminModoExcluir;
  adminUltimaAcao = millis();
  adminMensagemAtiva = false;
  bip(1500, 60);
  mostrarMenuAdmin();
  Serial.println(adminModoExcluir ? F("Admin: modo EXCLUIR") : F("Admin: modo CADASTRAR"));
}

// Segurar o botao MODO por 3 s: entra no admin (ou sai dele).
// Alternativa ao cartao ADMIN, caso a dupla tenha poucos cartoes.
void verificarPressaoLonga() {
  if (botaoModo.estadoEstavel == HIGH) {   // botao solto
    pressaoLongaUsada = false;
    return;
  }
  if (pressaoLongaUsada) return;
  if (millis() - botaoModo.ultimaMudanca < TEMPO_PRESSAO_LONGA) return;

  pressaoLongaUsada = true;
  if (estadoAtual == AGUARDANDO) {
    Serial.println(F("Botao MODO segurado: entrando no admin."));
    entrarAdmin();
  } else if (estadoAtual == MODO_ADMIN) {
    Serial.println(F("Botao MODO segurado: saindo do admin."));
    entrarAguardando();
  }
}

// Temporizadores da maquina de estados (tudo com millis, sem delay)
void atualizarEstado() {
  unsigned long agora = millis();
  unsigned long decorrido = agora - estadoDesde;

  switch (estadoAtual) {
    case PORTA_ABERTA:
      if (decorrido >= TEMPO_PORTA_ABERTA) entrarAguardando();
      break;

    case ACESSO_NEGADO:
      if (decorrido >= TEMPO_MSG_NEGADO) entrarAguardando();
      break;

    case BLOQUEADO: {
      if (decorrido >= TEMPO_BLOQUEIO) {
        Serial.println(F("Bloqueio encerrado."));
        entrarAguardando();
        break;
      }
      unsigned int restante = (TEMPO_BLOQUEIO - decorrido + 999UL) / 1000UL;
      if (restante != ultimoSegundoMostrado) {
        ultimoSegundoMostrado = restante;
        char linha2[17];
        snprintf(linha2, sizeof(linha2), "Aguarde %2us", restante);
        escreverLinha(1, linha2);
      }
      // LED vermelho pisca durante o bloqueio
      ligarLed(PINO_LED_VERMELHO, (decorrido / 500UL) % 2 == 0);
      break;
    }

    case MODO_ADMIN:
      if (agora - adminUltimaAcao >= TEMPO_ADMIN) {
        Serial.println(F("Admin: tempo esgotado."));
        entrarAguardando();
        break;
      }
      if (adminMensagemAtiva && agora - adminMensagemDesde >= TEMPO_MSG_ADMIN) {
        adminMensagemAtiva = false;
        mostrarMenuAdmin();
      }
      // LED verde pisca para indicar modo de manutencao
      ligarLed(PINO_LED_VERDE, (agora / 300UL) % 2 == 0);
      break;

    default:
      break;
  }
}

// ============================================================================
//  SETUP E LOOP
// ============================================================================
void setup() {
  Serial.begin(9600);

  pinMode(PINO_LED_VERDE, OUTPUT);
  pinMode(PINO_LED_VERMELHO, OUTPUT);
  pinMode(PINO_BUZZER, OUTPUT);
  apagarLeds();

  iniciarBotao(botaoModo);
#if MODO_SIMULACAO
  iniciarBotao(botaoSimAdmin);
  iniciarBotao(botaoSimCartao1);
  iniciarBotao(botaoSimCartao2);
#endif

  lcd.init();
  lcd.backlight();
  mostrarTela("  SMARTACCESS", " Iniciando...");

  servoTrava.attach(PINO_SERVO);
  servoTrava.write(ANGULO_FECHADO);

#if !MODO_SIMULACAO
  SPI.begin();
  leitorRfid.PCD_Init();
  byte versao = leitorRfid.PCD_ReadRegister(MFRC522::VersionReg);
  Serial.print(F("RC522 versao: 0x"));
  Serial.println(versao, HEX);
  if (versao == 0x00 || versao == 0xFF) {
    Serial.println(F("ERRO: RC522 nao respondeu. Verifique as ligacoes e o 3,3 V."));
    mostrarTela("ERRO NO RFID", "Verif. ligacoes");
    delay(3000);
  }
#endif

  // Segurar o botao MODO ao ligar apaga todos os cadastros
  if (digitalRead(PINO_BOTAO_MODO) == LOW) {
    apagarMemoria();
    mostrarTela("MEMORIA APAGADA", "Solte o botao");
    Serial.println(F("Cadastros apagados (botao MODO pressionado ao ligar)."));
    bip(1000, 500);
    while (digitalRead(PINO_BOTAO_MODO) == LOW) { }   // espera soltar
    delay(1000);
  }

  iniciarMemoria();
  delay(800);   // apenas para a tela de abertura aparecer

  Serial.println(F("=== SmartAccess iniciado ==="));
  listarCartoes();
  imprimirAjuda();

  entrarAguardando();
}

void loop() {
  if (foiPressionado(botaoModo)) tratarBotaoModo();
  verificarPressaoLonga();

  byte uid[4];
  byte tamanho = lerCartao(uid);
  if (tamanho > 0) tratarCartao(uid, tamanho);

  atualizarEstado();
}
