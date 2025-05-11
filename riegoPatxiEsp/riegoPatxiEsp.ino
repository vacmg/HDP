#include <Wire.h>
#include <RTClib.h> // Asegúrate de instalar la librería "RTClib by Adafruit"
#include <list>     // Incluir para usar std::list
#include <EEPROM.h> // Incluir para persistencia
#include <WiFi.h>   // Para WiFi
#include <WebServer.h> // Para Servidor Web
#include <WebSocketsServer.h> // <-- Para WebSockets
#include <ArduinoJson.h>      // <-- Para JSON

// --- Credenciales WiFi --- 
const char* WIFI_SSID = "patxi";
const char* WIFI_PASSWORD = "12345678";

// --- Definiciones de Pines ---
const int RELAY_VOLTAGE_PIN = 0;
const int RELAY_PUMP_PIN = 1;
const int RELAY_VALVE_PIN = 2;
const int WATER_LEVEL_PIN = 3;
const int VOLTAGE_SENSOR_PIN = 4;
const int RTC_SDA_PIN = 8;
const int RTC_SCL_PIN = 9;

// --- Constantes ---
const float VOLTAGE_SENSOR_SCALE_FACTOR = 4.39;
const long SERIAL_BAUD_RATE = 115200;
const unsigned long STATUS_PRINT_INTERVAL_MS = 5000;
const unsigned long VOLTAGE_CONTROL_INTERVAL_MS = 500;
const unsigned long PUMP_VOLTAGE_CONTROL_INTERVAL_MS = 500;
// const unsigned long VALVE_OPEN_DURATION_MS = 10000; // Eliminado, ahora configurable
const unsigned long INITIAL_PUMP_TIMEOUT_S = 60;
const unsigned long WIFI_CHECK_INTERVAL_MS = 10000;
const unsigned long WS_STATUS_UPDATE_INTERVAL_MS = 2000;
const float ADC_VREF = 3.3;
const int ADC_MAX_VALUE = 4095;

// --- Constantes de Comandos Serie ---
const char* CMD_PUMP_ON = "b1";
const char* CMD_PUMP_OFF = "b0";
const char* CMD_VALVE_ON = "a1";
const char* CMD_VALVE_OFF = "a0";
const char* CMD_STATUS = "s";
const char* CMD_HELP = "h";
const char* CMD_SET_VMIN_PREFIX = "vmin=";
const char* CMD_SET_VMAX_PREFIX = "vmax=";
const char* CMD_SET_PVMIN_PREFIX = "pvmin=";
const char* CMD_SET_PVMAX_PREFIX = "pvmax=";
const char* CMD_SET_MAXCYCLES_PREFIX = "maxcycles=";
const char* CMD_SET_PUMPTIMEOUT_PREFIX = "pumptimeout=";
const char* CMD_SET_VALVEDURATION_PREFIX = "valveduration=";
const char* CMD_IRRIGATION_ON = "r1";
const char* CMD_IRRIGATION_OFF = "r0";
const char* CMD_FORCE_CYCLE = "forcecycle";
const char* CMD_LIMIT_ON = "limit_on";
const char* CMD_LIMIT_OFF = "limit_off";
const char* CMD_RESET_DAY = "resetday";
const char* CMD_SCH_ADD_PREFIX = "schadd=";
const char* CMD_SCH_LIST = "schlist";
const char* CMD_SCH_DEL_PREFIX = "schdel=";
const char* CMD_SCH_ENABLE_PREFIX = "schen=";
const char* CMD_SCH_DISABLE_PREFIX = "schdis=";
const char* CMD_RESET_CONFIG = "resetconfig";
const char* CMD_SET_TIME_PREFIX = "settime=";

// --- Valores Iniciales / Por Defecto ---
const float INITIAL_VMIN = 8.0;
const float INITIAL_VMAX = 9.0;
const float INITIAL_PUMP_VMIN = 7.5;
const float INITIAL_PUMP_VMAX = 8.5;
const uint8_t INITIAL_MAX_CYCLES_PER_DAY = 5;
const bool INITIAL_DAILY_LIMIT_ENABLED = true;
const bool INITIAL_TIMEOUT_LOCKOUT = false;
const unsigned long INITIAL_VALVE_OPEN_DURATION_S = 10;

// --- Configuración EEPROM ---
const int EEPROM_ADDR = 0;
const uint32_t CONFIG_MAGIC_NUMBER = 0xDEA0BEEF;
const uint16_t CONFIG_VERSION = 1;
const uint8_t MAX_SCHEDULE_ENTRIES_EEPROM = 20;

// --- Estructura para Horarios ---
/** @brief Define una entrada de programación horaria. */
struct ScheduleEntry
{
    uint8_t daysOfWeekMask;
    uint8_t hour;
    uint8_t minute;
    bool activate;
    bool enabled;
};

// --- Estructura para Datos en EEPROM ---
/** @brief Estructura que agrupa toda la configuración persistente. */
struct ConfigData {
  uint32_t magicNumber;
  uint16_t version;
  float vMin;
  float vMax;
  float pumpVMin;
  float pumpVMax;
  uint8_t maxCyclesPerDay;
  unsigned long pumpTimeoutMs;
  bool dailyLimitEnabled;
  bool timeoutLockoutActive;
  unsigned long valveOpenDurationMs;
  uint8_t scheduleCount;
  uint8_t dayOfMonth;
  uint8_t cyclesToday;
  ScheduleEntry schedule[MAX_SCHEDULE_ENTRIES_EEPROM];
};

const int EEPROM_SIZE = sizeof(ConfigData);

// --- HTML Principal (en PROGMEM con CSS Interno) ---
const char PAGE_Root[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang='es'>
<head>
    <meta charset='UTF-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <title>Control Riego ESP32</title>
    <style>
        body { font-family: system-ui, -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, 'Open Sans', 'Helvetica Neue', sans-serif; background-color: #f1f5f9; padding: 1rem; color: #333; }
        .container { max-width: 900px; margin: 0 auto; background-color: #fff; padding: 1.5rem; border-radius: 0.5rem; box-shadow: 0 1px 3px 0 rgba(0, 0, 0, 0.1), 0 1px 2px -1px rgba(0, 0, 0, 0.1); }
        h1 { font-size: 1.5rem; font-weight: bold; margin-bottom: 1rem; text-align: center; color: #1e3a8a; }
        h2 { font-size: 1.25rem; font-weight: 600; margin-bottom: 0.75rem; color: #1e40af; border-bottom: 1px solid #e5e7eb; padding-bottom: 0.25rem; }
        h3 { font-size: 1rem; font-weight: 600; margin-bottom: 0.5rem; }
        .section { margin-bottom: 1.5rem; padding: 1rem; border: 1px solid #e5e7eb; border-radius: 0.5rem; }
        .grid-cols-2 { display: grid; grid-template-columns: repeat(2, minmax(0, 1fr)); gap: 1rem; }
        .status-label { font-weight: bold; min-width: 150px; display: inline-block; color: #4b5563; }
        .status-value { color: #1f2937; }
        .control-group { margin-bottom: 0.75rem; }
        .control-group label { margin-right: 0.5rem; display: inline-block; width: 130px; font-size: 0.9rem; }
        .control-group input[type='number'], .control-group input[type='text'], .control-group select {
            padding: 0.3rem 0.6rem; border: 1px solid #d1d5db; border-radius: 0.25rem; font-size: 0.9rem; max-width: 120px;
        }
        .btn { display: inline-block; padding: 0.5rem 1rem; border-radius: 0.375rem; cursor: pointer; margin: 0.25rem; font-size: 0.9rem; text-align: center; border: none; transition: background-color 0.2s ease; }
        .btn-sm { padding: 0.25rem 0.5rem; font-size: 0.8rem; margin-left: 0.25rem; }
        .btn-primary { background-color: #4f46e5; color: white; }
        .btn-secondary { background-color: #6b7280; color: white; }
        .btn-danger { background-color: #ef4444; color: white; }
        .btn:hover { filter: brightness(90%); }
        .flex-wrap { display: flex; flex-wrap: wrap; gap: 0.5rem; }
        #scheduleList div { border-bottom: 1px solid #eee; padding: 0.5rem 0; display: flex; justify-content: space-between; align-items: center; flex-wrap: wrap; gap: 0.5rem;}
        #scheduleList div:last-child { border-bottom: none; }
        #scheduleList span { flex-grow: 1; margin-right: 1rem; }
        @media (max-width: 768px) { .grid-cols-2 { grid-template-columns: repeat(1, minmax(0, 1fr)); } .control-group label { width: auto; } }
    </style>
</head>
<body>
<div class='container'>
    <h1>Control Riego ESP32</h1>

    <div class="grid-cols-2">
        <div>
            <div class='section'>
                <h2>Estado General</h2>
                <div><span class='status-label'>Hora RTC:</span><span id='rtcTime' class='status-value'>--</span></div>
                <div><span class='status-label'>Estado WiFi:</span><span id='wifiStatus' class='status-value'>--</span></div>
                <div><span class='status-label'>IP Local:</span><span id='wifiIP' class='status-value'>--</span></div>
                <div><span class='status-label'>Voltaje Sistema:</span><span id='voltage' class='status-value'>--</span> V</div>
                <div><span class='status-label'>Nivel Agua:</span><span id='waterLevel' class='status-value'>--</span></div>
            </div>

            <div class='section'>
                <h2>Estado Riego</h2>
                <div><span class='status-label'>Modo Riego:</span><span id='irrigationActive' class='status-value'>--</span></div>
                <div><span class='status-label'>Estado Actual:</span><span id='irrigationState' class='status-value'>--</span></div>
                <div><span class='status-label'>Ciclos Hoy:</span><span id='cyclesToday' class='status-value'>--</span> / <span id='maxCycles' class='status-value'>--</span></div>
                <div><span class='status-label'>Límite Diario:</span><span id='limitEnabled' class='status-value'>--</span></div>
                <div><span class='status-label'>Bloqueo por Timeout:</span><span id='timeoutLockout' class='status-value'>--</span></div>
                <div><span class='status-label'>Timeout Bomba:</span><span id='pumpTimeout' class='status-value'>--</span> s</div>
                <div><span class='status-label'>Duración Válvula:</span><span id='valveDuration' class='status-value'>--</span> s</div>
                <div id='pumpOnTimeDiv' style='display: none;'><span class='status-label'>Tiempo Bomba ON:</span><span id='pumpOnTime' class='status-value'>--</span> ms</div>
            </div>

            <div class='section'>
                <h2>Estado Relés</h2>
                <div><span class='status-label' id='relayVoltageName'>Relé Voltaje:</span><span id='relayVoltageState' class='status-value'>--</span></div>
                <div><span class='status-label' id='relayPumpName'>Relé Bomba:</span><span id='relayPumpState' class='status-value'>--</span></div>
                <div><span class='status-label' id='relayValveName'>Relé Válvula:</span><span id='relayValveState' class='status-value'>--</span></div>
            </div>
        </div>

        <div>
            <div class='section'>
                <h2>Controles Riego</h2>
                <div class='flex-wrap'>
                    <button id='btnStartIrrigation' class='btn btn-primary'>Iniciar Riego (r1)</button>
                    <button id='btnStopIrrigation' class='btn btn-danger'>Parar Riego (r0)</button>
                    <button id='btnForceCycle' class='btn btn-secondary'>Forzar Ciclo</button>
                    <button id='btnResetDay' class='btn btn-secondary'>Reset Diario</button>
                    <button id='btnLimitOn' class='btn btn-primary'>Activar Límite</button>
                    <button id='btnLimitOff' class='btn btn-secondary'>Desactivar Límite</button>
                    <button id='btnValveOn' class='btn btn-primary'>Cerrar Válvula (a1)</button>
                    <button id='btnValveOff' class='btn btn-secondary'>Abrir Válvula (a0)</button>
                </div>
            </div>

            <div class='section'>
                <h2>Configuración</h2>
                <div class='grid-cols-2'>
                    <div class='control-group'><label for='inpVMin'>VMin Carga:</label><input type='number' step='0.1' id='inpVMin'><button id='btnSetVMin' class='btn btn-secondary btn-sm'>Set</button></div>
                    <div class='control-group'><label for='inpVMax'>VMax Carga:</label><input type='number' step='0.1' id='inpVMax'><button id='btnSetVMax' class='btn btn-secondary btn-sm'>Set</button></div>
                    <div class='control-group'><label for='inpPVMin'>PVMin Bomba:</label><input type='number' step='0.1' id='inpPVMin'><button id='btnSetPVMin' class='btn btn-secondary btn-sm'>Set</button></div>
                    <div class='control-group'><label for='inpPVMax'>PVMax Bomba:</label><input type='number' step='0.1' id='inpPVMax'><button id='btnSetPVMax' class='btn btn-secondary btn-sm'>Set</button></div>
                    <div class='control-group'><label for='inpMaxCycles'>Max Ciclos/Día:</label><input type='number' step='1' id='inpMaxCycles'><button id='btnSetMaxCycles' class='btn btn-secondary btn-sm'>Set</button></div>
                    <div class='control-group'><label for='inpPumpTimeout'>Timeout Bomba(s):</label><input type='number' step='1' id='inpPumpTimeout'><button id='btnSetPumpTimeout' class='btn btn-secondary btn-sm'>Set</button></div>
                    <div class='control-group'><label for='inpValveDuration'>Duración Válvula(s):</label><input type='number' step='1' id='inpValveDuration'><button id='btnSetValveDuration' class='btn btn-secondary btn-sm'>Set</button></div>
                </div>
            </div>

             <div class='section'>
                <h2>Sistema</h2>
                 <div class='control-group'>
                    <label for='inpSetTime'>Ajustar Hora (YYYY,MM,DD,HH,MM,SS):</label>
                    <input type='text' id='inpSetTime' placeholder='2024,04,20,12,00,00'>
                    <button id='btnSetTime' class='btn btn-secondary btn-sm'>Ajustar</button>
                </div>
                <button id='btnResetConfig' class='btn btn-danger'>Reset Config Fábrica</button>
            </div>
        </div>

    </div> <div class='section'>
        <h2>Planificador (Máx: <span id='maxEntries'></span>)</h2>
        <div id='scheduleList' class='mb-4'>Cargando horarios...</div>
        <h3>Añadir / Modificar Horario</h3>
        <div class='flex flex-wrap items-center gap-2'>
             <input type='hidden' id='schEditIndex' value='-1'>
             <div class='control-group'><label for='schDays'>Días (LMXJVSD*):</label><input type='text' id='schDays' size='7'></div>
             <div class='control-group'><label for='schHour'>Hora (0-23):</label><input type='number' id='schHour' min='0' max='23'></div>
             <div class='control-group'><label for='schMinute'>Min (0-59):</label><input type='number' id='schMinute' min='0' max='59'></div>
             <div class='control-group'><label for='schAction'>Acción:</label><select id='schAction'><option value='1'>ON</option><option value='0'>OFF</option></select></div>
             <div class='control-group'><label for='schEnabled'>Estado:</label><select id='schEnabled'><option value='1'>Habilitado</option><option value='0'>Deshabilitado</option></select></div>
             <button id='schSaveBtn' class='btn btn-primary'>Guardar</button>
             <button id='schCancelBtn' style='display:none;' class='btn btn-secondary'>Cancelar Edición</button>
        </div>
    </div>

</div> <script>
const ws = new WebSocket('ws://' + window.location.hostname + ':81/');
let lastStatusData = {};

ws.onopen = () => { console.log('WebSocket conectado'); };
ws.onclose = () => { console.log('WebSocket desconectado'); document.getElementById('wifiStatus').innerText = 'Desconectado (WS)'; };
ws.onerror = (error) => { console.error('WebSocket Error:', error); };
ws.onmessage = (event) => {
    try {
        const data = JSON.parse(event.data);
        lastStatusData = data;
        updateStatus(data);
    } catch (e) { console.error('Error parseando JSON:', e); }
};

function updateStatus(data) {
  document.getElementById('rtcTime').innerText = data.rtcTime || '--';
  document.getElementById('wifiStatus').innerText = data.wifiStatus || '--';
  document.getElementById('wifiIP').innerText = data.wifiIP || '--';
  document.getElementById('voltage').innerText = data.voltage !== undefined ? data.voltage.toFixed(2) : '--';
  document.getElementById('waterLevel').innerText = data.waterLevel ? 'DETECTADA' : 'No detectada';
  document.getElementById('irrigationActive').innerText = data.irrigationActive ? 'ACTIVO' : 'INACTIVO';
  document.getElementById('irrigationState').innerText = data.irrigationState || '--';
  document.getElementById('cyclesToday').innerText = data.cyclesToday !== undefined ? data.cyclesToday : '--';
  document.getElementById('maxCycles').innerText = data.maxCycles !== undefined ? data.maxCycles : '--';
  document.getElementById('limitEnabled').innerText = data.limitEnabled ? 'Habilitado' : 'Deshabilitado';
  document.getElementById('timeoutLockout').innerText = data.timeoutLockout ? 'ACTIVO' : 'Inactivo';
  document.getElementById('pumpTimeout').innerText = data.pumpTimeoutS !== undefined ? data.pumpTimeoutS : '--';
  document.getElementById('valveDuration').innerText = data.valveDurationS !== undefined ? data.valveDurationS : '--';
  const pumpOnTimeDiv = document.getElementById('pumpOnTimeDiv');
  if(data.irrigationState === 'PUMPING') { document.getElementById('pumpOnTime').innerText = data.pumpOnTimeMs !== undefined ? data.pumpOnTimeMs : '--'; pumpOnTimeDiv.style.display = 'block'; } else { pumpOnTimeDiv.style.display = 'none'; }
  document.getElementById('relayVoltageName').innerText = (data.relayVoltageName || 'Relé Voltaje') + ':';
  document.getElementById('relayVoltageState').innerText = data.relayVoltageState ? 'ON (Corta)' : 'OFF (Permite)';
  document.getElementById('relayPumpName').innerText = (data.relayPumpName || 'Relé Bomba') + ':';
  document.getElementById('relayPumpState').innerText = data.relayPumpState ? 'ON' : 'OFF';
  document.getElementById('relayValveName').innerText = (data.relayValveName || 'Relé Válvula') + ':';
  document.getElementById('relayValveState').innerText = data.relayValveState ? 'ON (Cierra)' : 'OFF (Abre)';
  document.getElementById('maxEntries').innerText = data.maxEntries !== undefined ? data.maxEntries : '??';

  updateInputValueIfNotFocused('inpVMin', data.vMin, 2);
  updateInputValueIfNotFocused('inpVMax', data.vMax, 2);
  updateInputValueIfNotFocused('inpPVMin', data.pumpVMin, 2);
  updateInputValueIfNotFocused('inpPVMax', data.pumpVMax, 2);
  updateInputValueIfNotFocused('inpMaxCycles', data.maxCycles);
  updateInputValueIfNotFocused('inpPumpTimeout', data.pumpTimeoutS);
  updateInputValueIfNotFocused('inpValveDuration', data.valveDurationS);

  const scheduleDiv = document.getElementById('scheduleList');
  if(data.schedule) {
      scheduleDiv.innerHTML = '';
      if (data.schedule.length === 0) { scheduleDiv.innerHTML = '<p>No hay horarios programados.</p>'; }
      else {
          const daysToString = (mask) => { if (mask === 127) return '*'; let s = ''; if (mask & 1) s += 'D'; if (mask & 2) s += 'L'; if (mask & 4) s += 'M'; if (mask & 8) s += 'X'; if (mask & 16) s += 'J'; if (mask & 32) s += 'V'; if (mask & 64) s += 'S'; return s || '-'; };
          data.schedule.forEach((entry, index) => {
              const div = document.createElement('div');
              const daysStr = entry.daysMask !== undefined ? daysToString(entry.daysMask) : '--';
              div.innerHTML = `
                  <span>[${index}] ${daysStr} ${String(entry.hour).padStart(2,'0')}:${String(entry.minute).padStart(2,'0')} -> ${entry.action ? 'ON' : 'OFF'} (${entry.enabled ? 'Hab' : 'Des'})</span>
                  <div>
                      <button class='btn btn-secondary btn-sm' onclick='editSchedule(${index})'>Editar</button>
                      <button class='btn btn-danger btn-sm' onclick='deleteSchedule(${index})'>Borrar</button>
                      <button class='btn btn-secondary btn-sm' onclick='toggleSchedule(${index}, ${!entry.enabled})'>${entry.enabled ? 'Deshab' : 'Habil'}</button>
                  </div>`;
              scheduleDiv.appendChild(div);
          });
      }
  } else { scheduleDiv.innerText = 'Cargando...'; }
}

function updateInputValueIfNotFocused(elementId, value, precision = 0) {
    const inputElement = document.getElementById(elementId);
    if (document.activeElement !== inputElement && value !== undefined && value !== null) {
        inputElement.value = (typeof value === 'number' && precision > 0) ? value.toFixed(precision) : value;
    } else if (document.activeElement !== inputElement) { inputElement.value = ''; }
}

function sendWsCommand(cmd, value = null) { const payload = { command: cmd }; if (value !== null) payload.value = value; console.log('Enviando comando WS:', payload); ws.send(JSON.stringify(payload)); }
function sendWsDataCommand(cmd, data) { const payload = { command: cmd, data: data }; console.log('Enviando comando WS con datos:', payload); ws.send(JSON.stringify(payload)); }

document.getElementById('btnStartIrrigation').addEventListener('click', () => sendWsCommand('r1'));
document.getElementById('btnStopIrrigation').addEventListener('click', () => sendWsCommand('r0'));
document.getElementById('btnForceCycle').addEventListener('click', () => sendWsCommand('forcecycle'));
document.getElementById('btnResetDay').addEventListener('click', () => sendWsCommand('resetday'));
document.getElementById('btnLimitOn').addEventListener('click', () => sendWsCommand('limit_on'));
document.getElementById('btnLimitOff').addEventListener('click', () => sendWsCommand('limit_off'));
document.getElementById('btnValveOn').addEventListener('click', () => sendWsCommand('a1'));
document.getElementById('btnValveOff').addEventListener('click', () => sendWsCommand('a0'));
document.getElementById('btnSetVMin').addEventListener('click', () => { const val = document.getElementById('inpVMin').value; if(val !== '') sendWsCommand('set_vmin', parseFloat(val)); });
document.getElementById('btnSetVMax').addEventListener('click', () => { const val = document.getElementById('inpVMax').value; if(val !== '') sendWsCommand('set_vmax', parseFloat(val)); });
document.getElementById('btnSetPVMin').addEventListener('click', () => { const val = document.getElementById('inpPVMin').value; if(val !== '') sendWsCommand('set_pvmin', parseFloat(val)); });
document.getElementById('btnSetPVMax').addEventListener('click', () => { const val = document.getElementById('inpPVMax').value; if(val !== '') sendWsCommand('set_pvmax', parseFloat(val)); });
document.getElementById('btnSetMaxCycles').addEventListener('click', () => { const val = document.getElementById('inpMaxCycles').value; if(val !== '') sendWsCommand('set_maxcycles', parseInt(val)); });
document.getElementById('btnSetPumpTimeout').addEventListener('click', () => { const val = document.getElementById('inpPumpTimeout').value; if(val !== '') sendWsCommand('set_pumptimeout', parseInt(val)); });
document.getElementById('btnSetValveDuration').addEventListener('click', () => { const val = document.getElementById('inpValveDuration').value; if(val !== '') sendWsCommand('set_valveduration', parseInt(val)); });
document.getElementById('btnSetTime').addEventListener('click', () => { const val = document.getElementById('inpSetTime').value; if(val !== '') sendWsCommand('set_time', val); });
document.getElementById('btnResetConfig').addEventListener('click', () => { if(confirm('¿Seguro que quieres borrar la configuración y reiniciar?')) sendWsCommand('resetconfig'); });

function clearScheduleForm() { document.getElementById('schEditIndex').value = '-1'; document.getElementById('schDays').value = ''; document.getElementById('schHour').value = ''; document.getElementById('schMinute').value = ''; document.getElementById('schAction').value = '1'; document.getElementById('schEnabled').value = '1'; document.getElementById('schCancelBtn').style.display = 'none'; document.getElementById('schSaveBtn').innerText = 'Añadir Horario'; }
function editSchedule(index) { if (lastStatusData && lastStatusData.schedule && lastStatusData.schedule[index]) { const entry = lastStatusData.schedule[index]; document.getElementById('schEditIndex').value = index; document.getElementById('schDays').value = entry.daysMask !== undefined ? daysToString(entry.daysMask) : ''; document.getElementById('schHour').value = entry.hour; document.getElementById('schMinute').value = entry.minute; document.getElementById('schAction').value = entry.action ? '1' : '0'; document.getElementById('schEnabled').value = entry.enabled ? '1' : '0'; document.getElementById('schCancelBtn').style.display = 'inline-block'; document.getElementById('schSaveBtn').innerText = 'Guardar Cambios'; } }
function deleteSchedule(index) { if (confirm(`¿Seguro que quieres borrar el horario [${index}]?`)) { sendWsCommand('sch_del', index); } }
function toggleSchedule(index, enable) { sendWsCommand(enable ? 'sch_enable' : 'sch_disable', index); }

document.getElementById('schSaveBtn').addEventListener('click', () => {
    const index = parseInt(document.getElementById('schEditIndex').value);
    const scheduleData = {
        days: document.getElementById('schDays').value,
        hour: parseInt(document.getElementById('schHour').value),
        minute: parseInt(document.getElementById('schMinute').value),
        action: document.getElementById('schAction').value === '1',
        enabled: document.getElementById('schEnabled').value === '1'
    };
    if (!scheduleData.days || isNaN(scheduleData.hour) || isNaN(scheduleData.minute) || scheduleData.hour < 0 || scheduleData.hour > 23 || scheduleData.minute < 0 || scheduleData.minute > 59) {
        alert('Por favor, introduce datos válidos para el horario.');
        return;
    }

    if (index === -1) { // Añadir nuevo
        sendWsDataCommand('sch_add', scheduleData);
    } else { // Editar existente (Borrar + Añadir)
        console.log(`Editando índice ${index}: Borrando y añadiendo de nuevo.`);
        sendWsCommand('sch_del', index);
        setTimeout(() => {
             sendWsDataCommand('sch_add', scheduleData);
             clearScheduleForm();
        }, 100);
        return;
    }
    clearScheduleForm();
});

document.getElementById('schCancelBtn').addEventListener('click', clearScheduleForm);

const daysToString = (mask) => { if (mask === 127) return '*'; let s = ''; if (mask & 1) s += 'D'; if (mask & 2) s += 'L'; if (mask & 4) s += 'M'; if (mask & 8) s += 'X'; if (mask & 16) s += 'J'; if (mask & 32) s += 'V'; if (mask & 64) s += 'S'; return s || '-'; };

</script>
</div>
</body>
</html>
)rawliteral";

// --- Declaraciones Forward de Funciones ---
bool isValidUnsignedInt(const String& str);
bool isValidFloat(const String& str);
void saveConfiguration();
bool loadConfiguration();
void setup();
void loop();
void processThresholdCommand(String command);
void processIrrigationCommand(String command);
void processSchedulerCommand(String command);
void processGeneralCommand(String command);
void processSystemCommand(String command);
void processSerialCommand(String command);
void printHelp();
void printCurrentStatus(DateTime now);
void handleRoot();
void handleNotFound();
void checkWiFi();
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
void sendStatusUpdate();
void processWebSocketCommand(uint8_t num, const char* commandJson);

// --- Clase Relay ---
class Relay
{
private:
    int _pin;
    bool _activeHigh;
    bool _currentState;
    const char* _name;
    static constexpr const char* RELAY_TAG = "Relay";
public:
    Relay(int pin = -1, const char* name = "Unnamed Relay", bool activeHigh = true) :
        _pin(pin), _name(name), _activeHigh(activeHigh), _currentState(false)
    {}

    void begin()
    {
        if (_pin < 0) return;
        pinMode(_pin, OUTPUT);
        off();
        Serial.printf("[%s] Relé '%s' en pin %d inicializado (Active HIGH: %s)\n",
                      RELAY_TAG, _name, _pin, _activeHigh ? "Si" : "No");
    }
    void setState(bool state)
    {
        if (_pin < 0) return;
        if (state == _currentState) { return; }
        int pinLevel = (state == _activeHigh);
        digitalWrite(_pin, pinLevel);
        _currentState = state;
        Serial.printf("[%s] Relé '%s' %s (Pin %d -> %s)\n",
                      RELAY_TAG, _name, _currentState ? "ON" : "OFF", _pin, pinLevel ? "HIGH" : "LOW");
    }
    void on() { setState(true); }
    void off() { setState(false); }
    bool isOn() const { return _currentState; }
    const char* getName() const { return _name; }
};

// --- Clase WaterLevelSensor ---
class WaterLevelSensor
{
private:
    int _pin;
    bool _activeLow;
    const char* _name;
    static constexpr const char* SENSOR_TAG = "WaterSensor";
public:
    WaterLevelSensor(int pin, bool activeLow, const char* name = "Nivel Agua") :
        _pin(pin), _activeLow(activeLow), _name(name)
    {}

    void begin()
    {
        pinMode(_pin, _activeLow ? INPUT_PULLUP : INPUT_PULLDOWN);
        Serial.printf("[%s] Sensor '%s' en pin %d inicializado (Active LOW: %s)\n",
                      SENSOR_TAG, _name, _pin, _activeLow ? "Si" : "No");
    }
    bool getPinState() const { return digitalRead(_pin); }
    bool isWaterDetected() const
    {
        bool pinState = getPinState();
        return _activeLow ? !pinState : pinState;
    }
    const char* getName() const { return _name; }
};

// --- Clase VoltageSensor ---
class VoltageSensor
{
private:
    int _pin;
    float _scaleFactor;
    float _vRef;
    int _adcMaxValue;
    const char* _name;
    static constexpr const char* SENSOR_TAG = "VoltageSensor";
public:
    VoltageSensor(int pin, float scaleFactor, const char* name, float vRef = ADC_VREF, int adcMaxValue = ADC_MAX_VALUE) :
        _pin(pin), _scaleFactor(scaleFactor), _name(name), _vRef(vRef), _adcMaxValue(adcMaxValue)
    {}

    void begin()
    {
        pinMode(_pin, INPUT);
        Serial.printf("[%s] Sensor '%s' en pin %d inicializado (Scale: %.2f, VRef: %.1f, MaxADC: %d)\n",
                      SENSOR_TAG, _name, _pin, _scaleFactor, _vRef, _adcMaxValue);
    }
    int getRawValue() const { return analogRead(_pin); }
    float getVoltage() const
    {
        int raw = getRawValue();
        raw = constrain(raw, 0, _adcMaxValue);
        float voltage = (raw * _vRef / (float)_adcMaxValue) * _scaleFactor;
        return voltage;
    }
     const char* getName() const { return _name; }
};

// --- Clase VoltageController ---
class VoltageController
{
public:
    enum ForcedStateOnDisable { FORCE_OFF, FORCE_ON, LEAVE_AS_IS };

private:
    VoltageSensor& _sensor;
    Relay _relay;
    float _minThreshold;
    float _maxThreshold;
    unsigned long _controlInterval;
    unsigned long _lastControlTime;
    const char* _name;
    bool _controlLogicInverted;
    bool _isEnabled;
    static constexpr const char* CONTROLLER_TAG = "VoltageControl";

    void runControlLogic()
    {
        float currentVoltage = _sensor.getVoltage();
        bool targetState;

        if (currentVoltage < _minThreshold) { targetState = !_controlLogicInverted; }
        else if (currentVoltage > _maxThreshold) { targetState = _controlLogicInverted; }
        else { return; }

        if (_relay.isOn() != targetState)
        {
             Serial.printf("[%s] %s: V=%.2fV (Rango: %.2f-%.2f). Objetivo: %s.\n",
                           CONTROLLER_TAG, _name, currentVoltage, _minThreshold, _maxThreshold,
                           targetState ? "ON" : "OFF");
        }
        _relay.setState(targetState);
    }

public:
    VoltageController(VoltageSensor& sensor,
                      int relayPin, const char* relayName, bool relayActiveHigh,
                      float initialVMin, float initialVMax,
                      unsigned long interval, bool invertedLogic,
                      const char* controllerName = "Controlador Voltaje") :
        _sensor(sensor),
        _relay(relayPin, relayName, relayActiveHigh),
        _minThreshold(initialVMin),
        _maxThreshold(initialVMax),
        _controlInterval(interval),
        _lastControlTime(0),
        _name(controllerName),
        _controlLogicInverted(invertedLogic),
        _isEnabled(true)
    {}

    void begin()
    {
        _relay.begin();
    }

    void enable() { if (!_isEnabled) { Serial.printf("[%s] %s HABILITADO.\n", CONTROLLER_TAG, _name); _isEnabled = true; } }
    void disable(ForcedStateOnDisable finalState = FORCE_OFF)
    {
        if (_isEnabled)
        {
             Serial.printf("[%s] %s DESHABILITADO.\n", CONTROLLER_TAG, _name);
            _isEnabled = false;
            switch (finalState)
            {
                case FORCE_OFF: _relay.off(); break;
                case FORCE_ON: _relay.on(); break;
                case LEAVE_AS_IS: break;
            }
        }
    }
    bool isEnabled() const { return _isEnabled; }
    void update() { if (!_isEnabled) { return; } if (millis() - _lastControlTime >= _controlInterval) { _lastControlTime = millis(); runControlLogic(); } }

    bool setVMin(float newVMin)
    {
        if (newVMin > 0 && newVMin < _maxThreshold)
        {
            if (_minThreshold != newVMin)
            {
                 _minThreshold = newVMin;
                 Serial.printf("[%s] %s: vMin actualizado a: %.2f V\n", CONTROLLER_TAG, _name, _minThreshold);
                 return true;
            }
            return false;
        }
        else { Serial.printf("[%s] %s: Error: Valor inválido para vMin (%.2f). Debe ser > 0 y < vMax (%.2f).\n", CONTROLLER_TAG, _name, newVMin, _maxThreshold); return false; }
    }
    bool setVMax(float newVMax)
    {
        if (newVMax > 0 && newVMax > _minThreshold)
        {
             if (_maxThreshold != newVMax)
             {
                _maxThreshold = newVMax;
                Serial.printf("[%s] %s: vMax actualizado a: %.2f V\n", CONTROLLER_TAG, _name, _maxThreshold);
                return true;
             }
             return false;
        }
        else { Serial.printf("[%s] %s: Error: Valor inválido para vMax (%.2f). Debe ser > 0 y > vMin (%.2f).\n", CONTROLLER_TAG, _name, newVMax, _minThreshold); return false; }
    }
    float getVMin() const { return _minThreshold; }
    float getVMax() const { return _maxThreshold; }
    const char* getName() const { return _name; }
    bool isControlledRelayOn() const { return _relay.isOn(); }
    const char* getControlledRelayName() const { return _relay.getName(); }
};

// --- Clase IrrigationController ---
class IrrigationController
{
public:
    enum IrrigationState { IDLE, PREPARE_PUMPING, PUMPING, START_VALVE_WAIT, VALVE_WAIT, STOPPING };

private:
    Relay& _valveRelay;
    WaterLevelSensor& _levelSensor;
    VoltageController& _pumpVoltageController;

    IrrigationState _currentState;
    bool _isActive;
    unsigned long _valveOpenStartTime;
    unsigned long _valveOpenDurationMs; // Configurable
    uint8_t _cyclesToday;
    uint8_t _maxCyclesPerDay;
    bool _dailyLimitEnabled;
    bool _forceNextCycle;
    unsigned long _pumpingStartTime;
    unsigned long _pumpTimeoutDuration;
    unsigned long _accumulatedPumpOnTimeMs;
    unsigned long _lastUpdateTimeMs;
    bool _timeoutLockoutActive;
    const char* _name;
    static constexpr const char* CONTROLLER_TAG = "IrrigationCtrl";

    const char* getStateString(IrrigationState state)
    {
        switch (state)
        {
            case IDLE: return "IDLE"; case PREPARE_PUMPING: return "PREPARE_PUMPING"; case PUMPING: return "PUMPING";
            case START_VALVE_WAIT: return "START_VALVE_WAIT"; case VALVE_WAIT: return "VALVE_WAIT"; case STOPPING: return "STOPPING";
            default: return "UNKNOWN";
        }
    }
    void changeState(IrrigationState newState)
    {
        if (newState != _currentState)
        {
            Serial.printf("[%s] Cambiando estado de %s a %s\n", CONTROLLER_TAG, getStateString(_currentState), getStateString(newState));
            _currentState = newState;
        }
    }

public:
    IrrigationController(Relay& valve, WaterLevelSensor& sensor, VoltageController& pumpVoltageCtrl,
                         // unsigned long valveOpenDurationMs, // Ya no es const, se lee de EEPROM
                         uint8_t initialMaxCycles,
                         unsigned long initialPumpTimeoutS,
                         bool initialDailyLimitEnabled,
                         bool initialTimeoutLockout,
                         const char* name = "Control Riego") :
        _valveRelay(valve),
        _levelSensor(sensor),
        _pumpVoltageController(pumpVoltageCtrl),
        _currentState(IDLE),
        _isActive(false),
        _valveOpenStartTime(0),
        _valveOpenDurationMs(INITIAL_VALVE_OPEN_DURATION_S * 1000UL), // Valor inicial por defecto
        _cyclesToday(0),
        _maxCyclesPerDay(initialMaxCycles > 0 ? initialMaxCycles : 1),
        _dailyLimitEnabled(initialDailyLimitEnabled),
        _forceNextCycle(false),
        _pumpingStartTime(0),
        _pumpTimeoutDuration(initialPumpTimeoutS > 0 ? initialPumpTimeoutS * 1000UL : 60000UL),
        _accumulatedPumpOnTimeMs(0),
        _lastUpdateTimeMs(0),
        _timeoutLockoutActive(initialTimeoutLockout),
        _name(name)
    {}

    void begin()
    {
         _pumpVoltageController.disable(VoltageController::FORCE_OFF);
         _accumulatedPumpOnTimeMs = 0;
         _lastUpdateTimeMs = millis();
    }

    void start()
    {
        Serial.printf("[%s] Solicitud START recibida.\n", CONTROLLER_TAG);
        if (_timeoutLockoutActive) { Serial.printf("[%s] Riego bloqueado hoy debido a timeout previo.\n", CONTROLLER_TAG); return; }
        if (!_isActive)
        {
             _isActive = true;
             if (_currentState == IDLE) { changeState(PREPARE_PUMPING); }
             else { Serial.printf("[%s] Riego ya estaba en curso (estado %s), marcado como activo.\n", CONTROLLER_TAG, getStateString(_currentState)); }
        }
         else { Serial.printf("[%s] Riego ya estaba activo.\n", CONTROLLER_TAG); }
    }
    void stop()
    {
         Serial.printf("[%s] Solicitud STOP recibida.\n", CONTROLLER_TAG);
        if (_isActive)
        {
            _isActive = false;
            if (_currentState != IDLE && _currentState != STOPPING) { changeState(STOPPING); }
            else { Serial.printf("[%s] Riego ya estaba inactivo o parándose.\n", CONTROLLER_TAG); }
        }
        else { Serial.printf("[%s] Riego ya estaba inactivo.\n", CONTROLLER_TAG); }
    }

    void forceNextCycle()
    {
        Serial.printf("[%s] Forzando próximo ciclo de riego (ignora límite diario) e intentando iniciar...\n", CONTROLLER_TAG);
        if (_timeoutLockoutActive) { Serial.printf("[%s] ...pero el riego está bloqueado por timeout previo. Usa 'resetday' para desbloquear.\n", CONTROLLER_TAG); return; }
        _forceNextCycle = true;
        start();
    }

    bool enableDailyLimit(bool enable)
    {
        if (_dailyLimitEnabled != enable)
        {
            _dailyLimitEnabled = enable;
            Serial.printf("[%s] Límite de ciclos diarios %s.\n", CONTROLLER_TAG, enable ? "HABILITADO" : "DESHABILITADO");
            return true;
        }
        return false;
    }

    bool setMaxCyclesPerDay(uint8_t limit)
    {
        if (limit > 0)
        {
            if (_maxCyclesPerDay != limit)
            {
                 _maxCyclesPerDay = limit;
                 Serial.printf("[%s] Límite máximo de ciclos por día establecido a: %d\n", CONTROLLER_TAG, _maxCyclesPerDay);
                 return true;
            }
            return false;
        }
        else { Serial.printf("[%s] Error: Límite máximo de ciclos debe ser mayor que 0.\n", CONTROLLER_TAG); return false; }
    }

    bool resetDailyCounter()
    {
        bool changed = false;
        Serial.printf("[%s] Reseteando contador de ciclos diarios (era %d) y bloqueo por timeout.\n", CONTROLLER_TAG, _cyclesToday);
        _cyclesToday = 0;
        if (_timeoutLockoutActive)
        {
            _timeoutLockoutActive = false;
            Serial.printf("[%s] Bloqueo por timeout desactivado.\n", CONTROLLER_TAG);
            changed = true;
        }
        return changed;
    }

    bool setPumpTimeout(unsigned long timeoutSeconds)
    {
        if (timeoutSeconds > 0)
        {
            unsigned long newTimeoutMs = timeoutSeconds * 1000UL;
            if (_pumpTimeoutDuration != newTimeoutMs)
            {
                _pumpTimeoutDuration = newTimeoutMs;
                Serial.printf("[%s] Timeout de bomba actualizado a: %lu s\n", CONTROLLER_TAG, timeoutSeconds);
                return true;
            }
             return false;
        }
        else { Serial.printf("[%s] Error: Timeout de bomba debe ser mayor que 0 segundos.\n", CONTROLLER_TAG); return false; }
    }

    bool setValveOpenDuration(unsigned long durationSeconds)
    {
        if (durationSeconds > 0) {
            unsigned long newDurationMs = durationSeconds * 1000UL;
            if (_valveOpenDurationMs != newDurationMs) {
                _valveOpenDurationMs = newDurationMs;
                Serial.printf("[%s] Duración de válvula actualizada a: %lu s\n", CONTROLLER_TAG, durationSeconds);
                return true;
            }
            return false;
        } else {
            Serial.printf("[%s] Error: Duración de válvula debe ser mayor que 0 segundos.\n", CONTROLLER_TAG);
            return false;
        }
    }

    void applyTimeoutLockoutState(bool lockoutState)
    {
        if (_timeoutLockoutActive != lockoutState)
        {
             _timeoutLockoutActive = lockoutState;
             Serial.printf("[%s] Estado de bloqueo por timeout aplicado desde config: %s.\n", CONTROLLER_TAG, _timeoutLockoutActive ? "ACTIVO" : "Inactivo");
        }
    }

    void update()
    {
        unsigned long currentTimeMs = millis();
        unsigned long elapsedMs = currentTimeMs - _lastUpdateTimeMs;
        _lastUpdateTimeMs = currentTimeMs;

        if (!_isActive && _currentState != IDLE && _currentState != STOPPING) { changeState(STOPPING); }

        switch (_currentState)
        {
            case IDLE: break;
            case PREPARE_PUMPING:
                 if (_timeoutLockoutActive) { Serial.printf("[%s] Intento de inicio bloqueado por timeout previo.\n", CONTROLLER_TAG); stop(); break; }
                 if (_dailyLimitEnabled && !_forceNextCycle && _cyclesToday >= _maxCyclesPerDay) { Serial.printf("[%s] Límite de %d ciclos diarios alcanzado. Riego detenido para hoy.\n", CONTROLLER_TAG, _maxCyclesPerDay); stop(); break; }
                 Serial.printf("[%s] Estado PREPARE_PUMPING: Cerrando válvula, habilitando control bomba, reseteando timers (Ciclo %d/%d%s)...\n", CONTROLLER_TAG, _cyclesToday + 1, _maxCyclesPerDay, _forceNextCycle ? ", FORZADO" : "");
                 _valveRelay.on(); _pumpVoltageController.enable(); _pumpingStartTime = currentTimeMs; _accumulatedPumpOnTimeMs = 0; _forceNextCycle = false; changeState(PUMPING); break;
            case PUMPING:
                _pumpVoltageController.update();
                if (_pumpVoltageController.isControlledRelayOn()) { _accumulatedPumpOnTimeMs += elapsedMs; }
                if (_accumulatedPumpOnTimeMs >= _pumpTimeoutDuration)
                {
                    Serial.printf("[%s] ¡TIMEOUT! Bomba ON durante %lu ms sin detectar nivel (Límite: %lu ms). Bloqueando riego para hoy.\n", CONTROLLER_TAG, _accumulatedPumpOnTimeMs, _pumpTimeoutDuration);
                    _timeoutLockoutActive = true; saveConfiguration();
                    stop();
                }
                else if (_levelSensor.isWaterDetected())
                {
                    Serial.printf("[%s] Estado PUMPING: Nivel de agua detectado (Tiempo bomba ON: %lu ms).\n", CONTROLLER_TAG, _accumulatedPumpOnTimeMs);
                    _pumpVoltageController.disable(VoltageController::FORCE_OFF);
                    changeState(START_VALVE_WAIT);
                }
                break;
            case START_VALVE_WAIT:
                 Serial.printf("[%s] Estado START_VALVE_WAIT: Abriendo válvula, iniciando temporizador.\n", CONTROLLER_TAG);
                 _valveRelay.off(); _valveOpenStartTime = currentTimeMs; changeState(VALVE_WAIT); break;
            case VALVE_WAIT:
                if (currentTimeMs - _valveOpenStartTime >= _valveOpenDurationMs)
                {
                     Serial.printf("[%s] Estado VALVE_WAIT: Tiempo de válvula abierta (%lu ms) completado.\n", CONTROLLER_TAG, _valveOpenDurationMs);
                     _cyclesToday++; Serial.printf("[%s] Ciclo de riego %d/%d completado hoy.\n", CONTROLLER_TAG, _cyclesToday, _maxCyclesPerDay);
                     saveConfiguration();
                     if (_isActive) { changeState(PREPARE_PUMPING); } else { changeState(STOPPING); }
                }
                break;
            case STOPPING:
                Serial.printf("[%s] Estado STOPPING: Deshabilitando control bomba, abriendo válvula...\n", CONTROLLER_TAG);
                _pumpVoltageController.disable(VoltageController::FORCE_OFF); _valveRelay.off(); _accumulatedPumpOnTimeMs = 0; changeState(IDLE); break;
        }
    }

    void setCyclesToday(uint8_t cyclesToday) { _cyclesToday = cyclesToday; }
    bool isActive() const { return _isActive; }
    IrrigationState getCurrentState() const { return _currentState; }
    const char* getCurrentStateString() { return getStateString(_currentState); }
    const char* getName() const { return _name; }
    uint8_t getCyclesToday() const { return _cyclesToday; }
    uint8_t getMaxCyclesPerDay() const { return _maxCyclesPerDay; }
    bool isDailyLimitEnabled() const { return _dailyLimitEnabled; }
    bool isTimeoutLockoutActive() const { return _timeoutLockoutActive; }
    unsigned long getPumpTimeoutSeconds() const { return _pumpTimeoutDuration / 1000UL; }
    unsigned long getValveOpenDurationSeconds() const { return _valveOpenDurationMs / 1000UL; }
    unsigned long getAccumulatedPumpTimeMs() const { return _accumulatedPumpOnTimeMs; }
};

// --- Clase Scheduler ---
class Scheduler
{
public:
    static const uint8_t DOW_SUN = (1 << 0); static const uint8_t DOW_MON = (1 << 1); static const uint8_t DOW_TUE = (1 << 2);
    static const uint8_t DOW_WED = (1 << 3); static const uint8_t DOW_THU = (1 << 4); static const uint8_t DOW_FRI = (1 << 5);
    static const uint8_t DOW_SAT = (1 << 6); static const uint8_t DOW_ALL = 0b01111111;
    static const uint8_t MAX_ENTRIES = MAX_SCHEDULE_ENTRIES_EEPROM;

private:
    std::list<ScheduleEntry> _scheduleList;
    IrrigationController& _irrigationCtrl;
    uint8_t _lastMinuteChecked;
    const char* _name;
    static constexpr const char* SCHEDULER_TAG = "Scheduler";

    uint8_t parseDaysOfWeek(String dayString)
    {
        uint8_t mask = 0; dayString.toLowerCase(); if (dayString.indexOf('*') != -1) { return DOW_ALL; }
        if (dayString.indexOf('l') != -1) { mask |= DOW_MON; } if (dayString.indexOf('m') != -1) { mask |= DOW_TUE; }
        if (dayString.indexOf('x') != -1) { mask |= DOW_WED; } if (dayString.indexOf('j') != -1) { mask |= DOW_THU; }
        if (dayString.indexOf('v') != -1) { mask |= DOW_FRI; } if (dayString.indexOf('s') != -1) { mask |= DOW_SAT; }
        if (dayString.indexOf('d') != -1) { mask |= DOW_SUN; } return mask;
    }
    String daysMaskToString(uint8_t mask)
    {
        if (mask == DOW_ALL) { return "*"; } String s = "";
        if (mask & DOW_SUN) { s += "D"; } if (mask & DOW_MON) { s += "L"; } if (mask & DOW_TUE) { s += "M"; }
        if (mask & DOW_WED) { s += "X"; } if (mask & DOW_THU) { s += "J"; } if (mask & DOW_FRI) { s += "V"; }
        if (mask & DOW_SAT) { s += "S"; } if (s == "") { s = "-"; } return s;
    }
    void checkSchedule(DateTime now)
    {
        uint8_t currentDayOfWeek = now.dayOfTheWeek(); uint8_t currentHour = now.hour(); uint8_t currentMinute = now.minute(); uint8_t currentDayMask = (1 << currentDayOfWeek);
        bool actionTaken = false;
        for (const auto& entry : _scheduleList)
        {
            if (entry.enabled && entry.hour == currentHour && entry.minute == currentMinute && (entry.daysOfWeekMask & currentDayMask))
            {
                Serial.printf("[%s] ¡Coincidencia de horario! Días=%s, Hora=%02d:%02d, Acción=%s\n",
                            SCHEDULER_TAG, daysMaskToString(entry.daysOfWeekMask).c_str(), entry.hour, entry.minute, entry.activate ? "ON" : "OFF");
                if (entry.activate) { _irrigationCtrl.start(); } else { _irrigationCtrl.stop(); }
                actionTaken = true;
            }
        }
    }

public:
    Scheduler(IrrigationController& irrigationCtrl, const char* name = "Scheduler") :
        _irrigationCtrl(irrigationCtrl), _lastMinuteChecked(99), _name(name)
    {}
    void begin() { Serial.printf("[%s] %s inicializado.\n", SCHEDULER_TAG, _name); }
    bool addEntry(uint8_t daysMask, uint8_t hour, uint8_t minute, bool activate, bool enabled = true)
    {
        if (_scheduleList.size() >= MAX_ENTRIES) { Serial.printf("[%s] Error: No se pueden añadir más horarios (Límite: %d).\n", SCHEDULER_TAG, MAX_ENTRIES); return false; }
        if (daysMask == 0 || hour > 23 || minute > 59) { Serial.printf("[%s] Error al añadir entrada: Datos inválidos (mask=%d, h=%d, m=%d).\n", SCHEDULER_TAG, daysMask, hour, minute); return false; }
        _scheduleList.emplace_back(ScheduleEntry{daysMask, hour, minute, activate, enabled});
        const auto& addedEntry = _scheduleList.back();
        Serial.printf("[%s] Nueva entrada añadida: Días=%s, Hora=%02d:%02d, Acción=%s, Habilitado=%s\n",
                    SCHEDULER_TAG, daysMaskToString(addedEntry.daysOfWeekMask).c_str(), addedEntry.hour, addedEntry.minute, addedEntry.activate ? "ON" : "OFF", addedEntry.enabled ? "Si" : "No");
        return true;
    }
    bool addEntryFromString(String data)
    {
        int firstComma = data.indexOf(','); int secondComma = data.indexOf(',', firstComma + 1); int thirdComma = data.indexOf(',', secondComma + 1);
        if (firstComma == -1 || secondComma == -1 || thirdComma == -1) { Serial.printf("[%s] Error formato schadd: Se esperan 4 partes separadas por coma (DIAS,HH,MM,A). Recibido: %s\n", SCHEDULER_TAG, data.c_str()); return false; }
        String daysStr = data.substring(0, firstComma); int hourInt = data.substring(firstComma + 1, secondComma).toInt(); int minuteInt = data.substring(secondComma + 1, thirdComma).toInt(); bool activate = (data.substring(thirdComma + 1).toInt() == 1); uint8_t daysMask = parseDaysOfWeek(daysStr);
        if (daysMask == 0 && daysStr != "d" && daysStr != "D" && daysStr != "0") { Serial.printf("[%s] Error formato schadd: Días inválidos ('%s'). Usar L,M,X,J,V,S,D o *.\n", SCHEDULER_TAG, daysStr.c_str()); return false; }
        if (hourInt < 0 || hourInt > 23 || minuteInt < 0 || minuteInt > 59) { Serial.printf("[%s] Error formato schadd: Hora (%d) o minuto (%d) fuera de rango.\n", SCHEDULER_TAG, hourInt, minuteInt); return false; }
        return addEntry(daysMask, (uint8_t)hourInt, (uint8_t)minuteInt, activate);
    }
    void listEntries()
    {
        Serial.printf("\n---[%s] Lista de Horarios (%d entradas) ---\n", SCHEDULER_TAG, _scheduleList.size());
        if (_scheduleList.empty()) { Serial.println("  No hay horarios programados."); }
        else
        {
            int index = 0;
            for (const auto& entry : _scheduleList)
            {
                Serial.printf("  [%d] Días: %-7s | Hora: %02d:%02d | Acción: %s | Estado: %s\n",
                            index++, daysMaskToString(entry.daysOfWeekMask).c_str(), entry.hour, entry.minute,
                            entry.activate ? "ON " : "OFF", entry.enabled ? "Habilitado" : "Deshabilitado");
            }
        }
        Serial.println("------------------------------------------");
    }
    bool setEntryEnabled(int index, bool enable)
    {
        if (index < 0 || index >= _scheduleList.size()) { Serial.printf("[%s] Error: Índice %d fuera de rango (0-%d).\n", SCHEDULER_TAG, index, _scheduleList.size() - 1); return false; }
        std::list<ScheduleEntry>::iterator it = _scheduleList.begin(); std::advance(it, index);
        if (it->enabled != enable) { it->enabled = enable; Serial.printf("[%s] Entrada %d %s.\n", SCHEDULER_TAG, index, enable ? "habilitada" : "deshabilitada"); return true; }
        else { Serial.printf("[%s] Entrada %d ya estaba %s.\n", SCHEDULER_TAG, index, enable ? "habilitada" : "deshabilitada"); return false; }
    }
    bool deleteEntry(int index)
    {
         if (index < 0 || index >= _scheduleList.size()) { Serial.printf("[%s] Error: Índice %d fuera de rango (0-%d).\n", SCHEDULER_TAG, index, _scheduleList.size() - 1); return false; }
        std::list<ScheduleEntry>::iterator it = _scheduleList.begin(); std::advance(it, index);
        Serial.printf("[%s] Eliminando entrada %d: Días=%s, Hora=%02d:%02d, Acción=%s\n",
                    SCHEDULER_TAG, index, daysMaskToString(it->daysOfWeekMask).c_str(), it->hour, it->minute, it->activate ? "ON" : "OFF");
        _scheduleList.erase(it);
        return true;
    }
    void update(DateTime now) { uint8_t currentMinute = now.minute(); if (currentMinute != _lastMinuteChecked) { _lastMinuteChecked = currentMinute; checkSchedule(now); } }
    void clearSchedule() { _scheduleList.clear(); Serial.printf("[%s] Lista de horarios borrada.\n", SCHEDULER_TAG); }
    const std::list<ScheduleEntry>& getScheduleList() const { return _scheduleList; }
};


// --- Objetos Globales ---
RTC_DS3231 rtc;
Relay relayValve(RELAY_VALVE_PIN, "Valvula");
WaterLevelSensor waterSensor(WATER_LEVEL_PIN, true);
VoltageSensor voltageSensor(VOLTAGE_SENSOR_PIN, VOLTAGE_SENSOR_SCALE_FACTOR, "Voltaje");
VoltageController mainVoltageController(voltageSensor, RELAY_VOLTAGE_PIN, "Voltaje", true, INITIAL_VMIN, INITIAL_VMAX, VOLTAGE_CONTROL_INTERVAL_MS, true, "Ctrl Voltaje Carga");
VoltageController pumpVoltageController(voltageSensor, RELAY_PUMP_PIN, "Bomba", true, INITIAL_PUMP_VMIN, INITIAL_PUMP_VMAX, PUMP_VOLTAGE_CONTROL_INTERVAL_MS, true, "Ctrl Voltaje Bomba");
IrrigationController irrigationController(relayValve, waterSensor, pumpVoltageController, INITIAL_MAX_CYCLES_PER_DAY, INITIAL_PUMP_TIMEOUT_S, INITIAL_DAILY_LIMIT_ENABLED, INITIAL_TIMEOUT_LOCKOUT);
Scheduler scheduler(irrigationController);
WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
bool isWebServerRunning = false;
uint8_t lastDayOfMonth = 0;


// --- Funciones Auxiliares de Validación ---
bool isValidUnsignedInt(const String& str) {
    if (str.length() == 0) { return false; }
    for (int i = 0; i < str.length(); i++) {
        if (!isDigit(str[i])) { return false; }
    }
    return true;
}
bool isValidFloat(const String& str) {
    if (str.length() == 0) { return false; }
    bool pointSeen = false; bool digitSeen = false;
    for (int i = 0; i < str.length(); i++) {
        if (str[i] == '.') { if (pointSeen) { return false; } pointSeen = true; }
        else if (!isDigit(str[i])) { return false; }
        else { digitSeen = true; }
    }
    return digitSeen;
}


// --- Funciones de Configuración EEPROM ---
void saveConfiguration() {
    Serial.println(F("[CONFIG] Guardando configuración en EEPROM..."));
    ConfigData configToSave;

    configToSave.magicNumber = CONFIG_MAGIC_NUMBER;
    configToSave.version = CONFIG_VERSION;
    configToSave.vMin = mainVoltageController.getVMin();
    configToSave.vMax = mainVoltageController.getVMax();
    configToSave.pumpVMin = pumpVoltageController.getVMin();
    configToSave.pumpVMax = pumpVoltageController.getVMax();
    configToSave.maxCyclesPerDay = irrigationController.getMaxCyclesPerDay();
    configToSave.pumpTimeoutMs = irrigationController.getPumpTimeoutSeconds() * 1000UL;
    configToSave.valveOpenDurationMs = irrigationController.getValveOpenDurationSeconds() * 1000UL;
    configToSave.dailyLimitEnabled = irrigationController.isDailyLimitEnabled();
    configToSave.timeoutLockoutActive = irrigationController.isTimeoutLockoutActive();
    configToSave.dayOfMonth = rtc.now().day();
    configToSave.cyclesToday = irrigationController.getCyclesToday();

    const auto& scheduleList = scheduler.getScheduleList();
    configToSave.scheduleCount = 0;
    int i = 0;
    for (const auto& entry : scheduleList) {
        if (i >= MAX_SCHEDULE_ENTRIES_EEPROM) { Serial.printf("[CONFIG] Advertencia: Se superó el límite de %d horarios para guardar en EEPROM.\n", MAX_SCHEDULE_ENTRIES_EEPROM); break; }
        configToSave.schedule[i] = entry;
        i++;
    }
    configToSave.scheduleCount = i;
    Serial.printf("[CONFIG] %d horarios preparados para guardar.\n", configToSave.scheduleCount);

    EEPROM.put(EEPROM_ADDR, configToSave);
    if (EEPROM.commit()) { Serial.println(F("[CONFIG] Configuración guardada correctamente.")); }
    else { Serial.println(F("[CONFIG] ¡Error al guardar configuración en EEPROM!")); }
}
bool loadConfiguration() {
    Serial.println(F("[CONFIG] Cargando configuración desde EEPROM..."));
    ConfigData loadedConfig;
    EEPROM.get(EEPROM_ADDR, loadedConfig);

    if (loadedConfig.magicNumber != CONFIG_MAGIC_NUMBER || loadedConfig.version != CONFIG_VERSION) {
         Serial.println(F("[CONFIG] Datos inválidos o versión incorrecta. Usando valores por defecto y guardando."));
         scheduler.clearSchedule();
         mainVoltageController.setVMin(INITIAL_VMIN); mainVoltageController.setVMax(INITIAL_VMAX);
         pumpVoltageController.setVMin(INITIAL_PUMP_VMIN); pumpVoltageController.setVMax(INITIAL_PUMP_VMAX);
         irrigationController.setMaxCyclesPerDay(INITIAL_MAX_CYCLES_PER_DAY);
         irrigationController.setPumpTimeout(INITIAL_PUMP_TIMEOUT_S);
         irrigationController.setValveOpenDuration(INITIAL_VALVE_OPEN_DURATION_S);
         irrigationController.enableDailyLimit(INITIAL_DAILY_LIMIT_ENABLED);
         irrigationController.applyTimeoutLockoutState(INITIAL_TIMEOUT_LOCKOUT);
         lastDayOfMonth = rtc.now().day();
         saveConfiguration();
         return false;
    } else {
         Serial.println(F("[CONFIG] Configuración válida encontrada. Aplicando..."));
         mainVoltageController.setVMin(loadedConfig.vMin);
         mainVoltageController.setVMax(loadedConfig.vMax);
         pumpVoltageController.setVMin(loadedConfig.pumpVMin);
         pumpVoltageController.setVMax(loadedConfig.pumpVMax);
         irrigationController.setMaxCyclesPerDay(loadedConfig.maxCyclesPerDay);
         irrigationController.setPumpTimeout(loadedConfig.pumpTimeoutMs / 1000UL);
         irrigationController.setValveOpenDuration(loadedConfig.valveOpenDurationMs / 1000UL);
         irrigationController.enableDailyLimit(loadedConfig.dailyLimitEnabled);
         irrigationController.applyTimeoutLockoutState(loadedConfig.timeoutLockoutActive);
         irrigationController.setCyclesToday(loadedConfig.cyclesToday);
         lastDayOfMonth = loadedConfig.dayOfMonth;

         scheduler.clearSchedule();
         Serial.printf("[CONFIG] Cargando %d horarios desde EEPROM...\n", loadedConfig.scheduleCount);
         for (int i = 0; i < loadedConfig.scheduleCount; ++i) {
             if (i < MAX_SCHEDULE_ENTRIES_EEPROM && loadedConfig.schedule[i].hour <= 23 && loadedConfig.schedule[i].minute <= 59) {
                  scheduler.addEntry(loadedConfig.schedule[i].daysOfWeekMask, loadedConfig.schedule[i].hour,
                                     loadedConfig.schedule[i].minute, loadedConfig.schedule[i].activate,
                                     loadedConfig.schedule[i].enabled);
             } else { Serial.printf("[CONFIG] Error: Horario inválido encontrado en EEPROM índice %d. Ignorado.\n", i); }
         }
         return true;
     }
 }


// --- Manejadores de Peticiones Web y WebSocket ---
void handleRoot() {
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send_P(200, "text/html", PAGE_Root);
}
void handleNotFound() {
  String message = "Archivo No Encontrado\n\n";
  message += "URI: "; message += server.uri();
  message += "\nMetodo: "; message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArgumentos: "; message += server.args(); message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) { message += " " + server.argName(i) + ": " + server.arg(i) + "\n"; }
  server.send(404, "text/plain", message);
}

// --- Gestión WiFi y Servidor ---
void checkWiFi() {
    static unsigned long lastWifiCheckTime = 0;
    if (millis() - lastWifiCheckTime >= WIFI_CHECK_INTERVAL_MS) {
        lastWifiCheckTime = millis();
        if (WiFi.status() == WL_CONNECTED) {
            if (!isWebServerRunning) {
                Serial.println(F("[WebServer] Iniciando servidores..."));
                server.begin(); webSocket.begin(); webSocket.onEvent(webSocketEvent);
                isWebServerRunning = true; sendStatusUpdate();
            }
        } else {
            Serial.printf("[WiFi] Desconectado (Estado: %d)\n", WiFi.status());
            if (isWebServerRunning) { Serial.println(F("[WebServer] Deteniendo servidores...")); server.stop(); webSocket.close(); isWebServerRunning = false; }
            Serial.println(F("[WiFi] Intentando reconectar...")); WiFi.reconnect();
        }
    }
}

// --- Manejador de Eventos WebSocket ---
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED: Serial.printf("[WebSocket] Cliente #%u desconectado.\n", num); break;
        case WStype_CONNECTED: { IPAddress ip = webSocket.remoteIP(num); Serial.printf("[WebSocket] Cliente #%u conectado desde %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]); sendStatusUpdate(); } break;
        case WStype_TEXT: Serial.printf("[WebSocket] Mensaje de #%u: %s\n", num, payload); processWebSocketCommand(num, (const char*)payload); sendStatusUpdate(); break;
        case WStype_BIN: Serial.printf("[WebSocket] Binario de #%u recibido, longitud: %u\n", num, length); break;
        default: break;
    }
}

// --- Procesador de Comandos WebSocket ---
void processWebSocketCommand(uint8_t num, const char* commandJson) {
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, commandJson);
    if (error) { Serial.print(F("[WebSocket] Error parseando JSON: ")); Serial.println(error.c_str()); webSocket.sendTXT(num, "{\"status\":\"error\", \"message\":\"Invalid JSON\"}"); return; }
    const char* cmd = doc["command"];
    if (!cmd) { Serial.println(F("[WebSocket] Error: Comando JSON sin campo 'command'.")); webSocket.sendTXT(num, "{\"status\":\"error\", \"message\":\"Missing command field\"}"); return; }

    String commandStr = String(cmd);
    Serial.printf("[WebSocket] Comando recibido de #%u: '%s'\n", num, cmd);
    bool configChanged = false;

    if (commandStr == "r1") { irrigationController.start(); }
    else if (commandStr == "r0") { irrigationController.stop(); }
    else if (commandStr == "forcecycle") { irrigationController.forceNextCycle(); }
    else if (commandStr == "resetday") { irrigationController.stop(); configChanged = irrigationController.resetDailyCounter(); }
    else if (commandStr == "limit_on") { configChanged = irrigationController.enableDailyLimit(true); }
    else if (commandStr == "limit_off") { configChanged = irrigationController.enableDailyLimit(false); }
    else if (commandStr == "a1") { relayValve.on(); }
    else if (commandStr == "a0") { relayValve.off(); }
    else if (commandStr == "set_vmin") { if (doc.containsKey("value") && doc["value"].is<float>()) { configChanged = mainVoltageController.setVMin(doc["value"].as<float>()); } else { Serial.println(F("[WebSocket] Error: Falta o tipo incorrecto para 'value' en set_vmin")); } }
    else if (commandStr == "set_vmax") { if (doc.containsKey("value") && doc["value"].is<float>()) { configChanged = mainVoltageController.setVMax(doc["value"].as<float>()); } else { Serial.println(F("[WebSocket] Error: Falta o tipo incorrecto para 'value' en set_vmax")); } }
    else if (commandStr == "set_pvmin") { if (doc.containsKey("value") && doc["value"].is<float>()) { configChanged = pumpVoltageController.setVMin(doc["value"].as<float>()); } else { Serial.println(F("[WebSocket] Error: Falta o tipo incorrecto para 'value' en set_pvmin")); } }
    else if (commandStr == "set_pvmax") { if (doc.containsKey("value") && doc["value"].is<float>()) { configChanged = pumpVoltageController.setVMax(doc["value"].as<float>()); } else { Serial.println(F("[WebSocket] Error: Falta o tipo incorrecto para 'value' en set_pvmax")); } }
    else if (commandStr == "set_maxcycles") { if (doc.containsKey("value") && doc["value"].is<unsigned int>()) { configChanged = irrigationController.setMaxCyclesPerDay(doc["value"].as<unsigned int>()); } else { Serial.println(F("[WebSocket] Error: Falta o tipo incorrecto para 'value' en set_maxcycles")); } }
    else if (commandStr == "set_pumptimeout") { if (doc.containsKey("value") && doc["value"].is<unsigned long>()) { configChanged = irrigationController.setPumpTimeout(doc["value"].as<unsigned long>()); } else { Serial.println(F("[WebSocket] Error: Falta o tipo incorrecto para 'value' en set_pumptimeout")); } }
    else if (commandStr == "set_valveduration") { if (doc.containsKey("value") && doc["value"].is<unsigned long>()) { configChanged = irrigationController.setValveOpenDuration(doc["value"].as<unsigned long>()); } else { Serial.println(F("[WebSocket] Error: Falta o tipo incorrecto para 'value' en set_valveduration")); } }
    else if (commandStr == "sch_add") {
         if (doc.containsKey("data")) {
             JsonObject data = doc["data"];
             if (data.containsKey("days") && data.containsKey("hour") && data.containsKey("minute") && data.containsKey("action") && data.containsKey("enabled")) {
                 String daysStr = data["days"].as<String>(); uint8_t hour = data["hour"].as<uint8_t>(); uint8_t minute = data["minute"].as<uint8_t>(); bool activate = data["action"].as<bool>(); bool enabled = data["enabled"].as<bool>();
                 String commandSerial = String(CMD_SCH_ADD_PREFIX) + daysStr + "," + String(hour) + "," + String(minute) + "," + String(activate ? 1:0);
                 bool added = scheduler.addEntryFromString(commandSerial.substring(strlen(CMD_SCH_ADD_PREFIX)));
                 if (added) { configChanged = true; if (!enabled) { int lastIndex = scheduler.getScheduleList().size() - 1; if (lastIndex >= 0) { scheduler.setEntryEnabled(lastIndex, false); } } }
             } else { Serial.println(F("[WebSocket] Error: Faltan datos en 'data' para sch_add")); }
         } else { Serial.println(F("[WebSocket] Error: Falta campo 'data' para sch_add")); }
    }
    else if (commandStr == "sch_del") { if (doc.containsKey("value") && doc["value"].is<int>()) { configChanged = scheduler.deleteEntry(doc["value"].as<int>()); } else { Serial.println(F("[WebSocket] Error: Falta o tipo incorrecto para 'value' en sch_del")); } }
    else if (commandStr == "sch_enable") { if (doc.containsKey("value") && doc["value"].is<int>()) { configChanged = scheduler.setEntryEnabled(doc["value"].as<int>(), true); } else { Serial.println(F("[WebSocket] Error: Falta o tipo incorrecto para 'value' en sch_enable")); } }
    else if (commandStr == "sch_disable") { if (doc.containsKey("value") && doc["value"].is<int>()) { configChanged = scheduler.setEntryEnabled(doc["value"].as<int>(), false); } else { Serial.println(F("[WebSocket] Error: Falta o tipo incorrecto para 'value' en sch_disable")); } }
    else if (commandStr == "set_time") { if (doc.containsKey("value") && doc["value"].is<const char*>()) { processSystemCommand(String(CMD_SET_TIME_PREFIX) + doc["value"].as<const char*>()); } else { Serial.println(F("[WebSocket] Error: Falta o tipo incorrecto para 'value' en set_time")); } }
    else if (commandStr == "resetconfig") { processSystemCommand(String(CMD_RESET_CONFIG)); }
    else { Serial.printf("[WebSocket] Comando desconocido recibido: %s\n", cmd); webSocket.sendTXT(num, "{\"status\":\"error\", \"message\":\"Unknown command\"}"); return; }

    if (configChanged) { saveConfiguration(); }
    webSocket.sendTXT(num, "{\"status\":\"ok\"}");
}


// --- Envío de Estado por WebSocket ---
void sendStatusUpdate() {
    if (!isWebServerRunning) return;

    StaticJsonDocument<2048> jsonDoc;

    jsonDoc["rtcTime"] = rtc.now().timestamp(DateTime::TIMESTAMP_FULL);
    jsonDoc["wifiStatus"] = (WiFi.status() == WL_CONNECTED) ? "Conectado" : ("Desconectado (" + String(WiFi.status()) + ")");
    jsonDoc["wifiIP"] = (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString() : "-";
    jsonDoc["voltage"] = voltageSensor.getVoltage();
    jsonDoc["waterLevel"] = waterSensor.isWaterDetected();
    jsonDoc["irrigationActive"] = irrigationController.isActive();
    jsonDoc["irrigationState"] = irrigationController.getCurrentStateString();
    jsonDoc["cyclesToday"] = irrigationController.getCyclesToday();
    jsonDoc["maxCycles"] = irrigationController.getMaxCyclesPerDay();
    jsonDoc["limitEnabled"] = irrigationController.isDailyLimitEnabled();
    jsonDoc["timeoutLockout"] = irrigationController.isTimeoutLockoutActive();
    jsonDoc["pumpTimeoutS"] = irrigationController.getPumpTimeoutSeconds();
    jsonDoc["valveDurationS"] = irrigationController.getValveOpenDurationSeconds();
    if (irrigationController.getCurrentState() == IrrigationController::PUMPING) { jsonDoc["pumpOnTimeMs"] = irrigationController.getAccumulatedPumpTimeMs(); }
    jsonDoc["relayVoltageName"] = mainVoltageController.getControlledRelayName();
    jsonDoc["relayVoltageState"] = mainVoltageController.isControlledRelayOn();
    jsonDoc["relayPumpName"] = pumpVoltageController.getControlledRelayName();
    jsonDoc["relayPumpState"] = pumpVoltageController.isControlledRelayOn();
    jsonDoc["relayValveName"] = relayValve.getName();
    jsonDoc["relayValveState"] = relayValve.isOn();
    jsonDoc["vMin"] = mainVoltageController.getVMin();
    jsonDoc["vMax"] = mainVoltageController.getVMax();
    jsonDoc["pumpVMin"] = pumpVoltageController.getVMin();
    jsonDoc["pumpVMax"] = pumpVoltageController.getVMax();
    jsonDoc["maxEntries"] = Scheduler::MAX_ENTRIES;

    JsonArray scheduleArray = jsonDoc.createNestedArray("schedule");
    const auto& list = scheduler.getScheduleList();
    for(const auto& entry : list) {
        JsonObject scheduleObj = scheduleArray.createNestedObject();
        scheduleObj["daysMask"] = entry.daysOfWeekMask;
        scheduleObj["hour"] = entry.hour;
        scheduleObj["minute"] = entry.minute;
        scheduleObj["action"] = entry.activate;
        scheduleObj["enabled"] = entry.enabled;
    }

    String jsonString;
    serializeJson(jsonDoc, jsonString);
    webSocket.broadcastTXT(jsonString);
}


// --- Setup ---
void setup()
{
    Serial.begin(SERIAL_BAUD_RATE);
    if (!EEPROM.begin(EEPROM_SIZE)) { Serial.println("[ERROR] Fallo al inicializar EEPROM!"); }
    else { Serial.println("[INFO] EEPROM inicializada correctamente."); }

    delay(1000);
    Serial.println(F("\n---[MAIN] Inicializando Sistema ---"));

    relayValve.begin();
    waterSensor.begin();
    voltageSensor.begin();
    mainVoltageController.begin();
    pumpVoltageController.begin();

    Serial.printf("[MAIN] Inicializando I2C en pines SDA=%d, SCL=%d\n", RTC_SDA_PIN, RTC_SCL_PIN);
    if (!Wire.begin(RTC_SDA_PIN, RTC_SCL_PIN)) { Serial.println(F("[MAIN] ¡Error al inicializar I2C! Deteniendo.")); while(1) { mainVoltageController.update(); delay(100); } }
    else { Serial.println(F("[MAIN] I2C Inicializado correctamente.")); }

    Serial.print(F("[MAIN] Inicializando RTC DS3231..."));
    if (!rtc.begin()) { Serial.println(F(" ¡No se encontró el RTC!")); while(1) { mainVoltageController.update(); delay(100); } }
    else {
        Serial.println(F(" RTC encontrado."));
        if (rtc.lostPower()) {
            Serial.println(F("[MAIN] ¡RTC perdió energía! Ajustando hora a la de compilación."));
            rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        }
         Serial.println(F("[MAIN] RTC OK."));
    }

    irrigationController.begin();
    scheduler.begin();

    Serial.println(F("[MAIN] Hardware y Controladores inicializados."));

    loadConfiguration();

    Serial.printf("[WiFi] Conectando a: %s\n", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    server.on("/", HTTP_GET, handleRoot);
    server.onNotFound(handleNotFound);

    Serial.println(F("---[MAIN] Setup Completo ---"));
    Serial.printf("[MAIN] Umbrales Carga: vMin=%.2f V, vMax=%.2f V\n", mainVoltageController.getVMin(), mainVoltageController.getVMax());
    Serial.printf("[MAIN] Umbrales Bomba: pvMin=%.2f V, pvMax=%.2f V\n", pumpVoltageController.getVMin(), pumpVoltageController.getVMax());
    Serial.printf("[MAIN] Límite Riegos/Día: %d (%s)\n",
                  irrigationController.getMaxCyclesPerDay(),
                  irrigationController.isDailyLimitEnabled() ? "Habilitado" : "Deshabilitado");
    Serial.printf("[MAIN] Timeout Bomba: %lu s\n", irrigationController.getPumpTimeoutSeconds());
    Serial.printf("[MAIN] Duración Válvula: %lu s\n", irrigationController.getValveOpenDurationSeconds());
    printHelp();
    delay(3000);
}

// --- Loop ---
void loop()
{
    static unsigned long lastWsUpdate = 0;

    if(lastDayOfMonth != rtc.now().day())
    {
        Serial.printf("[MAIN] Dia nuevo detectado, poniendo cyclesToday = 0 y recargando configuracion\n");
        irrigationController.setCyclesToday(0);
        saveConfiguration();
        loadConfiguration();
    }

    checkWiFi();

    if (isWebServerRunning) {
        webSocket.loop();
        server.handleClient();
    }

    if (Serial.available() > 0)
    {
        String command = Serial.readStringUntil('\n');
        command.trim();
        Serial.printf("[MAIN] Comando recibido: %s\n", command.c_str());
        processSerialCommand(command);
        sendStatusUpdate();
    }

    DateTime now = rtc.now();

    mainVoltageController.update();
    irrigationController.update();
    scheduler.update(now);

    if (millis() - lastWsUpdate >= WS_STATUS_UPDATE_INTERVAL_MS) {
        lastWsUpdate = millis();
        sendStatusUpdate();
    }

    static unsigned long lastStatusPrintTime = 0;
    if (millis() - lastStatusPrintTime >= STATUS_PRINT_INTERVAL_MS)
    {
        lastStatusPrintTime = millis();
        printCurrentStatus(now);
    }

    delay(10);
}

// --- Procesamiento de Comandos Serie ---
/** @brief Procesa comandos relacionados con los umbrales de voltaje y límites. */
void processThresholdCommand(String command)
{
     bool configChanged = false;
     if (command.startsWith(CMD_SET_VMIN_PREFIX)) { String valueStr = command.substring(strlen(CMD_SET_VMIN_PREFIX)); if (isValidFloat(valueStr)) { configChanged = mainVoltageController.setVMin(valueStr.toFloat()); } else { Serial.printf("[MAIN] Error: Valor numérico inválido para %s: %s\n", CMD_SET_VMIN_PREFIX, valueStr.c_str()); } }
     else if (command.startsWith(CMD_SET_VMAX_PREFIX)) { String valueStr = command.substring(strlen(CMD_SET_VMAX_PREFIX)); if (isValidFloat(valueStr)) { configChanged = mainVoltageController.setVMax(valueStr.toFloat()); } else { Serial.printf("[MAIN] Error: Valor numérico inválido para %s: %s\n", CMD_SET_VMAX_PREFIX, valueStr.c_str()); } }
     else if (command.startsWith(CMD_SET_PVMIN_PREFIX)) { String valueStr = command.substring(strlen(CMD_SET_PVMIN_PREFIX)); if (isValidFloat(valueStr)) { configChanged = pumpVoltageController.setVMin(valueStr.toFloat()); } else { Serial.printf("[MAIN] Error: Valor numérico inválido para %s: %s\n", CMD_SET_PVMIN_PREFIX, valueStr.c_str()); } }
     else if (command.startsWith(CMD_SET_PVMAX_PREFIX)) { String valueStr = command.substring(strlen(CMD_SET_PVMAX_PREFIX)); if (isValidFloat(valueStr)) { configChanged = pumpVoltageController.setVMax(valueStr.toFloat()); } else { Serial.printf("[MAIN] Error: Valor numérico inválido para %s: %s\n", CMD_SET_PVMAX_PREFIX, valueStr.c_str()); } }
     else if (command.startsWith(CMD_SET_MAXCYCLES_PREFIX)) { String valueStr = command.substring(strlen(CMD_SET_MAXCYCLES_PREFIX)); if (isValidUnsignedInt(valueStr)) { configChanged = irrigationController.setMaxCyclesPerDay((uint8_t)valueStr.toInt()); } else { Serial.printf("[MAIN] Error: Valor numérico inválido para %s: %s\n", CMD_SET_MAXCYCLES_PREFIX, valueStr.c_str()); } }
     else if (command.startsWith(CMD_SET_PUMPTIMEOUT_PREFIX)) { String valueStr = command.substring(strlen(CMD_SET_PUMPTIMEOUT_PREFIX)); if (isValidUnsignedInt(valueStr)) { configChanged = irrigationController.setPumpTimeout(valueStr.toInt()); } else { Serial.printf("[MAIN] Error: Valor numérico inválido para %s: %s\n", CMD_SET_PUMPTIMEOUT_PREFIX, valueStr.c_str()); } }
     else if (command.startsWith(CMD_SET_VALVEDURATION_PREFIX)) { String valueStr = command.substring(strlen(CMD_SET_VALVEDURATION_PREFIX)); if (isValidUnsignedInt(valueStr)) { configChanged = irrigationController.setValveOpenDuration(valueStr.toInt()); } else { Serial.printf("[MAIN] Error: Valor numérico inválido para %s: %s\n", CMD_SET_VALVEDURATION_PREFIX, valueStr.c_str()); } }
     else { Serial.println(F("[MAIN] Comando desconocido (processThresholdCommand).")); }
     if (configChanged) { saveConfiguration(); }
}
/** @brief Procesa comandos relacionados con el control de riego y límites. */
void processIrrigationCommand(String command)
{
    bool configChanged = false;
    if (command == CMD_IRRIGATION_ON) { Serial.println(F("[MAIN] Comando para activar riego recibido.")); irrigationController.start(); }
    else if (command == CMD_IRRIGATION_OFF) { Serial.println(F("[MAIN] Comando para desactivar riego recibido.")); irrigationController.stop(); }
    else if (command == CMD_FORCE_CYCLE) { Serial.println(F("[MAIN] Forzando próximo ciclo de riego...")); irrigationController.forceNextCycle(); }
    else if (command == CMD_LIMIT_ON) { Serial.println(F("[MAIN] Habilitando límite de ciclos diarios...")); configChanged = irrigationController.enableDailyLimit(true); }
    else if (command == CMD_LIMIT_OFF) { Serial.println(F("[MAIN] Deshabilitando límite de ciclos diarios...")); configChanged = irrigationController.enableDailyLimit(false); }
    else if (command == CMD_RESET_DAY) { Serial.println(F("[MAIN] Reseteando contador de ciclos diarios y bloqueo por timeout...")); irrigationController.stop(); configChanged = irrigationController.resetDailyCounter(); }
    else { Serial.println(F("[MAIN] Comando desconocido (processIrrigationCommand).")); }
    if (configChanged) { saveConfiguration(); }
}
/** @brief Procesa comandos relacionados con el Scheduler. */
void processSchedulerCommand(String command)
{
    bool configChanged = false;
     if (command.startsWith(CMD_SCH_ADD_PREFIX)) { String data = command.substring(strlen(CMD_SCH_ADD_PREFIX)); configChanged = scheduler.addEntryFromString(data); }
    else if (command == CMD_SCH_LIST) { scheduler.listEntries(); }
    else if (command.startsWith(CMD_SCH_DEL_PREFIX)) { String valueStr = command.substring(strlen(CMD_SCH_DEL_PREFIX)); if (isValidUnsignedInt(valueStr)) { configChanged = scheduler.deleteEntry(valueStr.toInt()); } else { Serial.printf("[MAIN] Error: Índice inválido para %s: %s\n", CMD_SCH_DEL_PREFIX, valueStr.c_str()); } }
    else if (command.startsWith(CMD_SCH_ENABLE_PREFIX)) { String valueStr = command.substring(strlen(CMD_SCH_ENABLE_PREFIX)); if (isValidUnsignedInt(valueStr)) { configChanged = scheduler.setEntryEnabled(valueStr.toInt(), true); } else { Serial.printf("[MAIN] Error: Índice inválido para %s: %s\n", CMD_SCH_ENABLE_PREFIX, valueStr.c_str()); } }
    else if (command.startsWith(CMD_SCH_DISABLE_PREFIX)) { String valueStr = command.substring(strlen(CMD_SCH_DISABLE_PREFIX)); if (isValidUnsignedInt(valueStr)) { configChanged = scheduler.setEntryEnabled(valueStr.toInt(), false); } else { Serial.printf("[MAIN] Error: Índice inválido para %s: %s\n", CMD_SCH_DISABLE_PREFIX, valueStr.c_str()); } }
    else { Serial.println(F("[MAIN] Comando desconocido (processSchedulerCommand).")); }
    if (configChanged) { saveConfiguration(); }
}
/** @brief Procesa comandos generales o de control manual. */
void processGeneralCommand(String command)
{
    if (command == CMD_VALVE_ON) { Serial.println(F("[MAIN] Activando relé Válvula (cierra)...")); relayValve.on(); }
    else if (command == CMD_VALVE_OFF) { Serial.println(F("[MAIN] Desactivando relé Válvula (abre)...")); relayValve.off(); }
    else if (command == CMD_STATUS) { Serial.println(F("[MAIN] Mostrando estado...")); printCurrentStatus(rtc.now()); }
    else if (command == CMD_HELP) { Serial.println(F("[MAIN] Mostrando ayuda...")); printHelp(); }
    else if (command == CMD_PUMP_ON) { Serial.println(F("[MAIN] Activando relé Bomba (manual)...")); pumpVoltageController.disable(VoltageController::FORCE_ON); }
    else if (command == CMD_PUMP_OFF) { Serial.println(F("[MAIN] Desactivando relé Bomba (manual)...")); pumpVoltageController.disable(VoltageController::FORCE_OFF); }
    else { Serial.println(F("[MAIN] Comando desconocido.")); }
}
/** @brief Procesa comandos de sistema. */
void processSystemCommand(String command)
{
    if (command == CMD_RESET_CONFIG)
    {
        Serial.println(F("[SYS] *** ADVERTENCIA *** Invalidando configuración guardada en EEPROM y reiniciando..."));
        ConfigData tempConfig; EEPROM.get(EEPROM_ADDR, tempConfig); tempConfig.magicNumber = 0; tempConfig.version = 0; EEPROM.put(EEPROM_ADDR, tempConfig);
        if (EEPROM.commit()) { Serial.println(F("[SYS] Configuración invalidada en EEPROM. Reiniciando en 2 segundos...")); } else { Serial.println(F("[SYS] ¡ERROR al invalidar configuración en EEPROM! Reiniciando de todas formas...")); }
        delay(2000); ESP.restart();
    }
    else if (command.startsWith(CMD_SET_TIME_PREFIX))
    {
        String data = command.substring(strlen(CMD_SET_TIME_PREFIX)); Serial.printf("[SYS] Intentando ajustar hora a: %s\n", data.c_str());
        int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0; int partCount = 0; int lastComma = -1; int nextComma;
        for (int i = 0; i < 6; ++i) {
            nextComma = data.indexOf(',', lastComma + 1); if (nextComma == -1 && i < 5) { partCount = -1; break; }
            String part = (i < 5) ? data.substring(lastComma + 1, nextComma) : data.substring(lastComma + 1); part.trim();
            if (!isValidUnsignedInt(part)) { partCount = -1; break; } int val = part.toInt();
            switch (i) { case 0: year = val; break; case 1: month = val; break; case 2: day = val; break; case 3: hour = val; break; case 4: minute = val; break; case 5: second = val; break; }
            lastComma = nextComma; partCount++;
        }
        if (partCount != 6) { Serial.println(F("[SYS] Error: Formato incorrecto para settime. Usar: YYYY,MM,DD,HH,MM,SS")); }
        else if (year < 2024 || year > 2099 || month < 1 || month > 12 || day < 1 || day > 31 || hour > 23 || minute > 59 || second > 59) { Serial.println(F("[SYS] Error: Valor fuera de rango en fecha/hora.")); }
        else { DateTime newTime(year, month, day, hour, minute, second); if (!newTime.isValid()) { Serial.println(F("[SYS] Error: La fecha/hora resultante no es válida.")); } else { rtc.adjust(newTime); Serial.println(F("[SYS] Hora del RTC ajustada correctamente.")); printCurrentStatus(rtc.now()); } }
    }
    else { Serial.println(F("[MAIN] Comando desconocido (processSystemCommand).")); }
}
/** @brief Despachador principal para comandos serie. */
void processSerialCommand(String command)
{
    if (command.startsWith("vmin") || command.startsWith("vmax") || command.startsWith("pvmin") || command.startsWith("pvmax") || command.startsWith("maxcycles") || command.startsWith("pumptimeout") || command.startsWith("valveduration")) { processThresholdCommand(command); }
    else if (command.startsWith("r") || command.startsWith("force") || command.startsWith("limit") || command == CMD_RESET_DAY) { processIrrigationCommand(command); }
    else if (command.startsWith("sch")) { processSchedulerCommand(command); }
    else if (command == CMD_RESET_CONFIG || command.startsWith(CMD_SET_TIME_PREFIX)) { processSystemCommand(command); }
    else { processGeneralCommand(command); }
}


// --- Función para imprimir la Ayuda ---
void printHelp()
{
    Serial.println(F("\n---[HELP] Lista de Comandos Disponibles ---"));
    Serial.printf("  %s / %s : Activar / Desactivar Relé Bomba (Control Manual - Desactiva control automático)\n", CMD_PUMP_ON, CMD_PUMP_OFF);
    Serial.printf("  %s / %s : Activar(CIERRA) / Desactivar(ABRE) Relé Válvula (Control Manual)\n", CMD_VALVE_ON, CMD_VALVE_OFF);
    Serial.printf("  %s=valor  : Establecer umbral mínimo de voltaje de CARGA (Ej: vmin=8.1)\n", CMD_SET_VMIN_PREFIX);
    Serial.printf("  %s=valor  : Establecer umbral máximo de voltaje de CARGA (Ej: vmax=9.5)\n", CMD_SET_VMAX_PREFIX);
    Serial.printf("  %s=valor : Establecer umbral mínimo de voltaje de BOMBA (Ej: pvmin=7.6)\n", CMD_SET_PVMIN_PREFIX);
    Serial.printf("  %s=valor : Establecer umbral máximo de voltaje de BOMBA (Ej: pvmax=8.8)\n", CMD_SET_PVMAX_PREFIX);
    Serial.printf("  %s=N      : Establecer LÍMITE de ciclos de riego por día (Ej: maxcycles=5)\n", CMD_SET_MAXCYCLES_PREFIX);
    Serial.printf("  %s=SEG   : Establecer TIMEOUT de seguridad para bomba (segundos). Ej: pumptimeout=60\n", CMD_SET_PUMPTIMEOUT_PREFIX);
    Serial.printf("  %s=SEG   : Establecer DURACIÓN apertura válvula (segundos). Ej: valveduration=10\n", CMD_SET_VALVEDURATION_PREFIX);
    Serial.println(F("  --- Control Riego ---"));
    Serial.printf("  %s / %s : Activar / Desactivar modo Riego Automático\n", CMD_IRRIGATION_ON, CMD_IRRIGATION_OFF);
    Serial.printf("  %s / %s : Habilitar / Deshabilitar el límite de ciclos diarios\n", CMD_LIMIT_ON, CMD_LIMIT_OFF);
    Serial.printf("  %s    : Forzar inicio del próximo ciclo de riego (ignora límite, si no hay bloqueo por timeout)\n", CMD_FORCE_CYCLE);
    Serial.printf("  %s     : Resetear contador de ciclos diarios y bloqueo por timeout\n", CMD_RESET_DAY);
    Serial.println(F("  --- Horarios Riego ---"));
    Serial.printf("  %s=D,HH,MM,A : Añadir horario (D=Dias(LMXJVSD/*), HH=0-23, MM=0-59, A=1(ON)/0(OFF)). Ej: %slmx,08,30,1\n", CMD_SCH_ADD_PREFIX, CMD_SCH_ADD_PREFIX);
    Serial.printf("  %s          : Listar horarios programados\n", CMD_SCH_LIST);
    Serial.printf("  %s=INDEX    : Habilitar horario por índice (ver lista)\n", CMD_SCH_ENABLE_PREFIX);
    Serial.printf("  %s=INDEX    : Deshabilitar horario por índice\n", CMD_SCH_DISABLE_PREFIX);
    Serial.printf("  %s=INDEX    : Borrar horario por índice\n", CMD_SCH_DEL_PREFIX);
    Serial.println(F("  --- Sistema ---"));
    Serial.printf("  %s=YYYY,MM,DD,HH,MM,SS : Ajustar fecha y hora del RTC. Ej: %s2024,04,19,10,30,00\n", CMD_SET_TIME_PREFIX, CMD_SET_TIME_PREFIX);
    Serial.printf("  %s   : Borrar config. guardada y reiniciar con valores por defecto\n", CMD_RESET_CONFIG);
    Serial.println(F("  --- Otros ---"));
    Serial.printf("  %s       : Mostrar Estado Actual Sensores y Hora\n", CMD_STATUS);
    Serial.printf("  %s       : Mostrar esta ayuda\n", CMD_HELP);
    Serial.println(F("------------------------------------------"));
}

// --- Función para imprimir el estado actual ---
void printCurrentStatus(DateTime now)
{
    bool waterDetected = waterSensor.isWaterDetected();
    float currentVoltage = voltageSensor.getVoltage();

    Serial.println(F("\n---[STATUS] Estado Actual ---"));

    char datetimeBuffer[25];
    snprintf(datetimeBuffer, sizeof(datetimeBuffer), "%04d/%02d/%02d %02d:%02d:%02d",
             now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
    Serial.printf("[STATUS] Fecha y Hora (RTC): %s (DoW: %d)\n", datetimeBuffer, now.dayOfTheWeek());

    Serial.print(F("[STATUS] Estado WiFi: "));
    if (WiFi.status() == WL_CONNECTED) {
        Serial.print(F("Conectado, IP: "));
        Serial.println(WiFi.localIP());
    } else {
        Serial.print(F("Desconectado (Estado: "));
        Serial.print(WiFi.status());
        Serial.println(")");
    }

    Serial.printf("[STATUS] %s: %s\n", waterSensor.getName(), waterDetected ? "AGUA DETECTADA" : "Sin agua");
    Serial.printf("[STATUS] %s: %.2f V\n", voltageSensor.getName(), currentVoltage);
    Serial.printf("[STATUS] Umbrales Carga: vMin=%.2f V, vMax=%.2f V\n", mainVoltageController.getVMin(), mainVoltageController.getVMax());
    Serial.printf("[STATUS] Umbrales Bomba: pvMin=%.2f V, pvMax=%.2f V\n", pumpVoltageController.getVMin(), pumpVoltageController.getVMax());

    Serial.printf("[STATUS] %s: %s (Estado: %s)\n",
                  irrigationController.getName(),
                  irrigationController.isActive() ? "ACTIVO" : "INACTIVO",
                  irrigationController.getCurrentStateString());
    Serial.printf("[STATUS] Ciclos Hoy: %d/%d (Límite %s)\n",
                  irrigationController.getCyclesToday(),
                  irrigationController.getMaxCyclesPerDay(),
                  irrigationController.isDailyLimitEnabled() ? "Habilitado" : "Deshabilitado");
    Serial.printf("[STATUS] Timeout Bomba: %lu s\n", irrigationController.getPumpTimeoutSeconds());
    Serial.printf("[STATUS] Duración Válvula: %lu s\n", irrigationController.getValveOpenDurationSeconds());
    Serial.printf("[STATUS] Bloqueo por Timeout: %s\n", irrigationController.isTimeoutLockoutActive() ? "ACTIVO" : "Inactivo");

    if (irrigationController.getCurrentState() == IrrigationController::PUMPING) {
        Serial.printf("[STATUS] Tiempo Bomba ON Acumulado: %lu ms\n", irrigationController.getAccumulatedPumpTimeMs());
    }

    Serial.println(F("---[STATUS] Estado Lógico Relés ---"));
    Serial.printf("[STATUS] %s: %s\n", mainVoltageController.getControlledRelayName(), mainVoltageController.isControlledRelayOn() ? "ON (Corriente Cortada)" : "OFF (Corriente Permitiendo)");
    Serial.printf("[STATUS] %s: %s\n", pumpVoltageController.getControlledRelayName(), pumpVoltageController.isControlledRelayOn() ? "ON" : "OFF");
    Serial.printf("[STATUS] %s: %s\n", relayValve.getName(), relayValve.isOn() ? "ON (Válvula Cerrada)" : "OFF (Válvula Abierta)");
    Serial.println(F("------------------------"));
}
