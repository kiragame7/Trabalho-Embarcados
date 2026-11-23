# Inventário: o que temos e o que falta

Atualizado com base nas fotos das peças do laboratório (kit UNISATC).

## ✅ Já temos

| Peça | Situação | Observação |
|---|---|---|
| Arduino Uno | ✅ | Na caixa de madeira |
| Protoboard 830 furos | ✅ | Na caixa de madeira |
| Chaveiros RFID azuis | ✅ (3 ou 4) | Servem como chefe, autorizado e desconhecido |
| Display LCD 16x2 **com módulo I2C** | ✅ | Tem a plaquinha preta atrás com GND, VCC, SDA, SCL. É o modelo certo. |
| Leitor RFID RC522 | ✅ | **Os 8 pinos já estão soldados**: não precisa soldar |
| Servo SG90 (Tower Pro) | ✅ (2) | Usamos 1, o outro é reserva. Tem as pecinhas brancas (braço). |
| Buzzer | ✅ (2) | Cilindros pretos. Usamos 1. |

## ❌ Peças que NÃO servem (já testadas nas fotos)

| Peça | Por quê |
|---|---|
| 2 "LEDs" **pretos** | São **receptores de infravermelho**: não acendem |
| 1 "LED" **rosado/transparente** | É **emissor de infravermelho**: acende, mas a luz é **invisível** (como controle remoto de TV). Dá para ver pela câmera do celular. |
| Sensores azuis com furinhos | Sensor de temperatura/umidade (DHT11): não usamos |
| Sensores com "dois olhos" | Sensor de distância (HC-SR04): não usamos |
| Bolinhas com fiozinho | Sensor de luz (LDR): não usamos |

## ⏳ Ainda falta

| Peça | Importância | Observação |
|---|---|---|
| **Fios jumper macho-fêmea** (~15) | ⭐ **Essencial** | Para ligar o display e o leitor RFID |
| **Fios jumper macho-macho** (~10) | ⭐ **Essencial** | Ligações na protoboard |
| **Cabo USB** do Arduino | ⭐ **Essencial** | Para gravar o programa |
| LED de **4 perninhas branco/fosco** (RGB) | Opcional | Tinha na caixa de plástico, canto de baixo à direita. Um só faz verde e vermelho. |
| Ou: 1 LED vermelho + 1 LED verde comuns | Opcional | Alternativa ao RGB |
| 2 resistores de 220 Ω a 1 kΩ | Opcional | Só se usar LED |
| 1 botão (push button) | Opcional | Pode trocar por 2 fios encostando um no outro |

### Se não acharmos LED
O projeto continua cumprindo todos os requisitos:
- **Saída digital** → buzzer e servo
- **Display** → LCD
- **Aviso de liberado/negado** → mensagem na tela + bip diferente

O código funciona normalmente com os pinos dos LEDs desligados.

## Para a maquete (opcional)
- Papelão grosso ou MDF
- Cola quente ou fita dupla face
- Palito de picolé (lingueta da trava no braço do servo)
