// ============================================
// Общие вспомогательные функции
// ============================================

function fetchJSON(url, options = {}) {
  return fetch(url, options)
    .then(response => {
      if (!response.ok) {
        if (response.status === 409) {
          return response.text().then(text => {
            console.log('409 response:', text);
            if (text.includes('Calibration required') || text.includes('Калибровка не выполнена')) {
              window.location.href = '/calibrate';
              return Promise.reject(new Error('Redirecting to calibration'));
            }
            throw new Error(text);
          });
        }
        return response.text().then(text => { throw new Error(text); });
      }
      const contentType = response.headers.get('content-type');
      if (contentType && contentType.includes('application/json')) {
        return response.json();
      }
      return response.text();
    });
}

// ============================================
// Переводы
// ============================================

const translations = {
  en: {
    back: "Home",
    saving: "Saving...",
    error: "Error: ",
    title: "Automatic Fertilizer Dispenser",
    turnOn: "ON",
    turnOff: "OFF",
    wifiSettings: "WiFi Settings",
    calibration: "Calibration",
    schedule: "Schedule",
    time: "Time",
    mqtt: "MQTT",
    config: "Configuration",
    diagnostics: "Diagnostics",
    statusPump: "Pump is",
    statusLevel: "Level:",
    statusCalibration: "Calibration:",
    statusMQTT: "MQTT:",
    levelOk: "OK",
    levelLow: "LOW!",
    mqttConnected: "Connected",
    mqttDisconnected: "Disconnected",
    calOk: "OK",
    calNotDone: "NOT DONE",
    wifiTitle: "WiFi Configuration",
    scanNetworks: "Scan Networks",
    selectNetwork: "-- Select network --",
    ssid: "SSID (home network):",
    password: "Home network password:",
    apPassword: "AP password (leave empty for default):",
    useStaticIP: "Use static IP",
    staticIP: "IP Address:",
    gateway: "Gateway:",
    subnet: "Subnet Mask:",
    dns1: "DNS 1 (optional):",
    dns2: "DNS 2 (optional):",
    saveRestart: "Save and Restart",
    resetWiFi: "Reset WiFi Settings",
    calibrationDesc: "Enter target volume (ml), start pump, and stop when reached.",
    targetVolume: "Target volume (ml):",
    startCalibration: "Start Calibration",
    stopCalibration: "Stop Calibration",
    notCalibratingStatus: "Waiting",
    calibratingStatus: "In progress",
    addJob: "Add New Job",
    jobTimeLabel: "Time:",
    volumeMl: "Volume (ml):",
    days: "Days:",
    enabled: "Enabled",
    existingJobs: "Existing Jobs",
    id: "ID",
    time: "Time",
    actions: "Actions",
    manualRun: "Manual Run (ml):",
    runNow: "Run Now",
    mon: "Mon",
    tue: "Tue",
    wed: "Wed",
    thu: "Thu",
    fri: "Fri",
    sat: "Sat",
    sun: "Sun",
    flowRateWarning: "Warning: Flow rate not calibrated! Please calibrate first.",
    syncNTP: "Sync with NTP",
    ntpServer: "NTP Server:",
    timezone: "Time zone (offset from UTC):",
    saveNTPServer: "Save NTP Server",
    manualSet: "Manual Set",
    year: "Year:",
    month: "Month:",
    day: "Day:",
    hour2: "Hour:",
    minute2: "Minute:",
    second: "Second:",
    setTime: "Set Time",
    mqttServer: "Server (IP or hostname):",
    mqttPort: "Port:",
    mqttUser: "Username (optional):",
    mqttPassword: "Password (optional):",
    mqttPrefix: "Topic prefix:",
    mqttLang: "Language:",
    mqttSave: "Save",
    mqttMissing: "Please fill server, port and prefix",
    mqttSaveSuccess: "Configuration saved, device restarting...",
    discoveryTitle: "Discovery",
    sendDiscovery: "Send MQTT Discovery",
    ledBrightness: "Indicator Brightness:",
    setBrightness: "Set",
    selectDays: "Please select at least one day",
    levelSensorType: "Sensor active level:",
    levelActiveHigh: "HIGH",
    levelActiveLow: "LOW",
    setLevelConfig: "Apply",
    duration: "Duration",
    calibStandard: "Standard",
    calibMulti: "Multi (3 measurements)",
    calibFixed: "Fixed time",
    calibManual: "Manual input",
    multiDesc: "Perform 3 measurements. After each, press 'Stop calibration'.",
    resetResults: "Reset",
    fixedTimeLabel: "Time (sec):",
    startFixed: "Start",
    fixedVolumeLabel: "Collected volume (ml):",
    saveFixed: "Save",
    manualFlowLabel: "Flow rate (ml/s):",
    saveManual: "Save",
    results: "Results:",
    fixedSaved: "Flow rate saved",
    manualSaved: "Flow rate saved",
    averageSaved: "Averaged flow rate saved:",
    maxMeasurementsReached: "Maximum 3 measurements reached. Start a new series by pressing 'Reset'.",
    exportConfig: "Export",
    importConfig: "Import",
    exportSuccess: "Configuration exported",
    importError: "Import failed: ",
    importSuccess: "Configuration imported. Device will restart in 3 seconds...",
    fixedRemaining: "Remaining: {0} sec"
  },
  ru: {
    back: "На главную",
    saving: "Сохранение...",
    error: "Ошибка: ",
    title: "Автоматический дозатор удобрений",
    turnOn: "ВКЛ",
    turnOff: "ВЫКЛ",
    wifiSettings: "Настройки WiFi",
    calibration: "Калибровка",
    schedule: "Расписание",
    time: "Время",
    mqtt: "MQTT",
    config: "Конфигурация",
    diagnostics: "Диагностика",
    statusPump: "Насос:",
    statusLevel: "Уровень:",
    statusCalibration: "Калибровка:",
    statusMQTT: "MQTT:",
    levelOk: "НОРМА",
    levelLow: "НИЗКИЙ",
    mqttConnected: "Подключён",
    mqttDisconnected: "Отключён",
    calOk: "ОК",
    calNotDone: "НЕ ВЫПОЛНЕНА",
    wifiTitle: "Настройки WiFi",
    scanNetworks: "Сканировать сети",
    selectNetwork: "-- Выберите сеть --",
    ssid: "SSID (домашней сети):",
    password: "Пароль домашней сети:",
    apPassword: "Пароль точки доступа (оставьте пустым для стандартного):",
    useStaticIP: "Использовать статический IP",
    staticIP: "IP-адрес:",
    gateway: "Шлюз:",
    subnet: "Маска подсети:",
    dns1: "DNS 1 (необязательно):",
    dns2: "DNS 2 (необязательно):",
    saveRestart: "Сохранить и перезагрузить",
    resetWiFi: "Сбросить настройки WiFi",
    calibrationDesc: "Введите целевой объём (мл), запустите насос и остановите, когда будет достигнут нужный объём.",
    targetVolume: "Целевой объём (мл):",
    startCalibration: "Старт калибровки",
    stopCalibration: "Стоп калибровки",
    notCalibratingStatus: "Ожидаем",
    calibratingStatus: "Выполняется",
    addJob: "Добавить задание",
    jobTimeLabel: "Время:",
    volumeMl: "Объём (мл):",
    days: "Дни недели:",
    enabled: "Активно",
    existingJobs: "Существующие задания",
    id: "ID",
    time: "Время",
    actions: "Действия",
    manualRun: "Ручной запуск (мл):",
    runNow: "Запустить",
    mon: "Пн",
    tue: "Вт",
    wed: "Ср",
    thu: "Чт",
    fri: "Пт",
    sat: "Сб",
    sun: "Вс",
    flowRateWarning: "Внимание: калибровка не выполнена! Пожалуйста, сначала выполните калибровку.",
    syncNTP: "Синхронизировать с NTP",
    ntpServer: "NTP-сервер:",
    timezone: "Часовой пояс (смещение от UTC):",
    saveNTPServer: "Сохранить NTP-сервер",
    manualSet: "Ручная установка",
    year: "Год:",
    month: "Месяц:",
    day: "День:",
    hour2: "Час:",
    minute2: "Минута:",
    second: "Секунда:",
    setTime: "Установить время",
    mqttServer: "Сервер (IP или имя):",
    mqttPort: "Порт:",
    mqttUser: "Пользователь (необязательно):",
    mqttPassword: "Пароль (необязательно):",
    mqttPrefix: "Префикс топиков:",
    mqttLang: "Язык:",
    mqttSave: "Сохранить",
    mqttMissing: "Заполните сервер, порт и префикс",
    mqttSaveSuccess: "Конфигурация сохранена, устройство перезагружается...",
    discoveryTitle: "Обнаружение",
    sendDiscovery: "Отправить MQTT Discovery",
    ledBrightness: "Яркость индикации:",
    setBrightness: "Установить",
    selectDays: "Выберите хотя бы один день",
    levelSensorType: "Активный уровень датчика:",
    levelActiveHigh: "HIGH",
    levelActiveLow: "LOW",
    setLevelConfig: "Применить",
    duration: "Длительность",
    calibStandard: "Обычная",
    calibMulti: "Многократная (3 замера)",
    calibFixed: "Фиксированное время",
    calibManual: "Ручной ввод",
    multiDesc: "Выполните 3 замера. После каждого нажимайте 'Стоп калибровки'.",
    resetResults: "Сбросить",
    fixedTimeLabel: "Время (сек):",
    startFixed: "Запустить",
    fixedVolumeLabel: "Собранный объём (мл):",
    saveFixed: "Сохранить",
    manualFlowLabel: "Расход (мл/с):",
    saveManual: "Сохранить",
    results: "Результаты:",
    fixedSaved: "Расход сохранён",
    manualSaved: "Расход сохранён",
    averageSaved: "Усреднённый расход сохранён:",
    maxMeasurementsReached: "Достигнуто максимальное количество замеров (3). Начните новую серию, нажав 'Сбросить'.",
    exportConfig: "Экспорт",
    importConfig: "Импорт",
    exportSuccess: "Конфигурация экспортирована",
    importError: "Ошибка импорта: ",
    importSuccess: "Конфигурация импортирована. Устройство перезагрузится через 3 секунды...",
    fixedRemaining: "Осталось: {0} сек"
  }
};

let currentLang = localStorage.getItem('lang') || 'ru';

function applyLanguage(lang) {
  currentLang = lang;
  localStorage.setItem('lang', lang);
  document.querySelectorAll('[data-i18n]').forEach(el => {
    const key = el.getAttribute('data-i18n');
    if (translations[lang][key]) {
      el.innerText = translations[lang][key];
    }
  });
  document.querySelectorAll('.lang-btn').forEach(btn => {
    btn.classList.toggle('active', btn.dataset.lang === lang);
  });
  if (typeof updateMQTTStatus === 'function') updateMQTTStatus();
  if (typeof updateCurrentTime === 'function') updateCurrentTime();
}

// ============================================
// WebSocket соединение и обработка статуса
// ============================================

let socket;
let reconnectInterval = 3000;

function connectWebSocket() {
  const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
  const wsUrl = protocol + '//' + window.location.hostname + ':81';
  socket = new WebSocket(wsUrl);

  socket.onopen = function() {
    console.log('WebSocket connected');
  };

  socket.onmessage = function(event) {
    try {
      const data = JSON.parse(event.data);
      updateStatus(data);
    } catch (e) {
      console.error('Invalid JSON:', event.data);
    }
  };

  socket.onclose = function() {
    console.log('WebSocket disconnected, reconnecting...');
    setTimeout(connectWebSocket, reconnectInterval);
  };

  socket.onerror = function(error) {
    console.error('WebSocket error:', error);
    socket.close();
  };
}

function updateStatus(data) {
  console.log('Full status object:', JSON.stringify(data));
  console.log('Status update (parsed):', data);

  const statusEl = document.querySelector('#status .value');
  const levelEl = document.querySelector('#levelStatus .value');
  const mqttEl = document.querySelector('#mqttStatus .value');
  const calEl = document.querySelector('#calibrationStatus .value');
  const dispenseInput = document.getElementById('dispenseVolume');
  const togglePumpBtn = document.getElementById('togglePumpBtn');

  if (statusEl) {
    statusEl.innerText = data.pump === 'ON' 
      ? translations[currentLang].turnOn 
      : translations[currentLang].turnOff;
  }
  if (levelEl) {
    levelEl.innerText = data.levelOk 
      ? translations[currentLang].levelOk 
      : translations[currentLang].levelLow;
    levelEl.style.color = data.levelOk ? '#28a745' : '#dc3545';
  }
  if (mqttEl) {
    const isConnected = data.mqtt === 'connected';
    mqttEl.innerText = isConnected 
      ? translations[currentLang].mqttConnected 
      : translations[currentLang].mqttDisconnected;
    mqttEl.style.color = isConnected ? '#28a745' : '#dc3545';
  }
  if (calEl) {
    calEl.innerText = data.calibrated 
      ? translations[currentLang].calOk 
      : translations[currentLang].calNotDone;
    calEl.style.color = data.calibrated ? '#28a745' : '#dc3545';
  }
  
  if (togglePumpBtn) {
    togglePumpBtn.innerText = data.pump === 'ON' ? translations[currentLang].turnOff : translations[currentLang].turnOn;
    togglePumpBtn.classList.remove('btn-on', 'btn-off');
    togglePumpBtn.classList.add(data.pump === 'ON' ? 'btn-on' : 'btn-off');
  }
  window.pumpState = data.pump === 'ON';

  if (data.pump === 'OFF' && dispenseInput) {
    dispenseInput.value = 0;
    console.log('Dispense volume reset to 0 (via WebSocket)');
  }

  const progressContainer = document.getElementById('progressContainer');
  const progressBar = document.getElementById('progressBar');
  const progressText = document.getElementById('progressText');

  if (progressContainer) {
    if (data.pump === 'OFF') {
      progressContainer.style.display = 'none';
    } else if (data.dispenseActive) {
      const elapsed = data.dispenseElapsed;
      const total = data.dispenseTotal;
      const percent = Math.min(100, (elapsed / total) * 100);
      const remainingSec = Math.floor((total - elapsed) / 1000);
      
      progressContainer.style.display = 'block';
      if (progressBar) progressBar.style.width = percent + '%';
      if (progressText) progressText.innerText = (currentLang === 'ru' 
        ? `Дозирование... Осталось: ${remainingSec} с` 
        : `Dispensing... Remaining: ${remainingSec} s`);
    } else {
      progressContainer.style.display = 'none';
    }
  }
}

// ============================================
// Управление насосом (вызовы API)
// ============================================

function turnOn() {
  fetchJSON('/pump/on', { method: 'POST' })
    .catch(err => {
      if (!err.message.includes('Redirecting')) {
        alert(translations[currentLang].error + err.message);
      }
    });
}

function turnOff() {
  fetchJSON('/pump/off', { method: 'POST' })
    .then(() => {
      const dispenseInput = document.getElementById('dispenseVolume');
      if (dispenseInput) {
        dispenseInput.value = 0;
        console.log('Dispense volume reset to 0 (via turnOff)');
      }
    })
    .catch(err => {
      if (!err.message.includes('Redirecting')) {
        alert(translations[currentLang].error + err.message);
      }
    });
}

function togglePump() {
  if (window.pumpState) {
    turnOff();
  } else {
    turnOn();
  }
}

function manualDispense() {
  const volume = document.getElementById('dispenseVolume').value;
  if (!volume || volume <= 0) {
    alert(translations[currentLang].manualRun);
    return;
  }
  fetchJSON('/schedule/run?volume=' + encodeURIComponent(volume), { method: 'POST' })
    .catch(err => {
      if (!err.message.includes('Redirecting')) {
        alert(translations[currentLang].error + err.message);
      }
    });
}

// ============================================
// Управление яркостью
// ============================================
function loadBrightness() {
  fetchJSON('/api/brightness')
    .then(data => {
      const slider = document.getElementById('brightnessSlider');
      const valueSpan = document.getElementById('brightnessValue');
      if (slider && valueSpan) {
        const percent = Math.round(data.brightness * 100 / 255);
        slider.value = percent;
        valueSpan.innerText = percent;
      }
    })
    .catch(err => console.error('Failed to load brightness', err));
}

function setBrightness() {
  const slider = document.getElementById('brightnessSlider');
  if (!slider) return;
  const percent = slider.value;
  fetchJSON('/api/brightness?value=' + percent, { method: 'POST' })
    .catch(err => alert(translations[currentLang].error + err.message));
}

document.addEventListener('input', function(e) {
  if (e.target.id === 'brightnessSlider') {
    document.getElementById('brightnessValue').innerText = e.target.value;
  }
});

// ============================================
// Настройка активного уровня датчика
// ============================================
function loadLevelConfig() {
  fetchJSON('/api/level/config')
    .then(data => {
      const select = document.getElementById('levelActiveLevel');
      if (select) {
        select.value = data.activeLevel;
      }
    })
    .catch(err => console.error('Failed to load level config', err));
}

function setLevelConfig() {
  const select = document.getElementById('levelActiveLevel');
  if (!select) return;
  const activeLevel = select.value;
  const params = new URLSearchParams();
  params.append('activeLevel', activeLevel);

  fetchJSON('/api/level/config', { method: 'POST', body: params })
    .catch(err => alert(translations[currentLang].error + err.message));
}

// ============================================
// WiFi страница
// ============================================

function scanNetworks() {
  const btn = document.getElementById('scanBtn');
  const select = document.getElementById('ssidSelect');
  if (!btn) return;
  btn.disabled = true;
  btn.innerHTML = translations[currentLang].scanNetworks + ' <span class="loader"></span>';

  fetchJSON('/scan')
    .then(data => {
      select.innerHTML = '<option value="">' + (currentLang === 'ru' ? '-- Выберите сеть --' : '-- Select network --') + '</option>';
      if (data.length > 0) {
        data.forEach(net => {
          if (net.ssid && net.ssid.length > 0) {
            const option = document.createElement('option');
            option.value = net.ssid;
            option.text = net.ssid + ' (' + net.rssi + ' dBm)';
            select.appendChild(option);
          }
        });
      } else {
        select.innerHTML = '<option value="">' + (currentLang === 'ru' ? 'Сети не найдены' : 'No networks found') + '</option>';
      }
    })
    .catch(err => {
      if (!err.message.includes('Redirecting')) {
        alert(translations[currentLang].error + err.message);
      }
    })
    .finally(() => {
      btn.disabled = false;
      btn.innerHTML = translations[currentLang].scanNetworks;
    });
}

function setSSID() {
  const select = document.getElementById('ssidSelect');
  const input = document.getElementById('ssid');
  if (select.value) input.value = select.value;
}

function toggleStaticIP(show) {
  document.getElementById('staticFields').style.display = show ? 'block' : 'none';
}

function validateIP(ip) {
  const pattern = /^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/;
  return pattern.test(ip);
}

function loadWiFiConfig() {
  fetchJSON('/wifi/get')
    .then(data => {
      document.getElementById('ssid').value = data.ssid;
      document.getElementById('password').value = data.password;
      document.getElementById('apPassword').value = data.apPassword;
      document.getElementById('useStaticIP').checked = data.useStaticIP;
      document.getElementById('staticIP').value = data.staticIP;
      document.getElementById('gateway').value = data.gateway;
      document.getElementById('subnet').value = data.subnet;
      document.getElementById('dns1').value = data.dns1;
      document.getElementById('dns2').value = data.dns2;
      toggleStaticIP(data.useStaticIP);
    })
    .catch(err => console.error('Failed to load WiFi config', err));
}

function saveWiFiConfig() {
  const ssid = document.getElementById('ssid').value;
  const password = document.getElementById('password').value;
  const apPassword = document.getElementById('apPassword').value;
  const useStaticIP = document.getElementById('useStaticIP').checked;
  const staticIP = document.getElementById('staticIP').value;
  const gateway = document.getElementById('gateway').value;
  const subnet = document.getElementById('subnet').value;
  const dns1 = document.getElementById('dns1').value;
  const dns2 = document.getElementById('dns2').value;

  if (!ssid || !password) {
    alert(translations[currentLang].ssid + ' ' + translations[currentLang].password);
    return;
  }

  if (useStaticIP) {
    if (!staticIP || !gateway || !subnet) {
      alert(translations[currentLang].staticIP);
      return;
    }
    if (!validateIP(staticIP) || !validateIP(gateway) || !validateIP(subnet)) {
      alert(translations[currentLang].staticIP);
      return;
    }
    if (dns1 && !validateIP(dns1)) {
      alert(translations[currentLang].dns1);
      return;
    }
    if (dns2 && !validateIP(dns2)) {
      alert(translations[currentLang].dns2);
      return;
    }
  }

  const params = new URLSearchParams();
  params.append('ssid', ssid);
  params.append('password', password);
  params.append('apPassword', apPassword);
  params.append('useStaticIP', useStaticIP ? '1' : '0');
  params.append('staticIP', staticIP);
  params.append('gateway', gateway);
  params.append('subnet', subnet);
  params.append('dns1', dns1);
  params.append('dns2', dns2);

  const btn = document.getElementById('saveBtn');
  const originalText = btn.innerHTML;
  btn.disabled = true;
  btn.innerHTML = translations[currentLang].saving + ' <span class="loader"></span>';

  fetchJSON('/saveconfig', { method: 'POST', body: params })
    .then(() => {})
    .catch(err => {
      if (!err.message.includes('Redirecting')) {
        alert(translations[currentLang].error + err.message);
        btn.disabled = false;
        btn.innerHTML = originalText;
      }
    });
}

function resetWiFiConfig() {
  if (confirm(translations[currentLang].resetWiFi)) {
    fetchJSON('/resetconfig', { method: 'POST' })
      .catch(err => {
        if (!err.message.includes('Redirecting')) {
          alert(translations[currentLang].error + err.message);
        }
      });
  }
}

// ============================================
// Калибровка (обновлённая)
// ============================================

let calibrationInterval = null;
let calibrationResults = [];

function loadCalibrationStatus() {
  fetchJSON('/calibrate/status')
    .then(data => {
      const toggleBtn = document.getElementById('toggleCalibBtn');
      const statusDiv = document.getElementById('status');
      const resultDiv = document.getElementById('result');
      const fixedVolumeBlock = document.getElementById('fixedVolumeBlock');
      const fixedProgress = document.getElementById('fixedProgress');

      if (data.calibrating) {
        if (toggleBtn) {
          toggleBtn.innerText = translations[currentLang].stopCalibration;
          toggleBtn.classList.remove('btn-off');
          toggleBtn.classList.add('btn-on');
        }
        if (statusDiv) statusDiv.innerText = translations[currentLang].calibratingStatus;
        
        // Если фиксированная калибровка — скрываем поле ввода объёма
        if (data.fixed && fixedVolumeBlock) {
          fixedVolumeBlock.style.display = 'none';
        }
        // Обновление оставшегося времени
        if (data.fixed && data.remainingSec > 0 && fixedProgress) {
          fixedProgress.innerText = translations[currentLang].fixedRemaining.replace('{0}', data.remainingSec);
        } else if (fixedProgress) {
          fixedProgress.innerText = '';
        }
        
        if (calibrationInterval) clearInterval(calibrationInterval);
        calibrationInterval = setInterval(updateCalibrationTime, 500);
      } else {
        if (toggleBtn) {
          toggleBtn.innerText = translations[currentLang].startCalibration;
          toggleBtn.classList.remove('btn-on');
          toggleBtn.classList.add('btn-off');
        }
        if (statusDiv) statusDiv.innerText = translations[currentLang].notCalibratingStatus;
        if (resultDiv) {
          if (data.flowRate > 0) {
            resultDiv.innerHTML = (currentLang === 'ru' ? 'Последняя калибровка: ' : 'Last calibrated flow rate: ') + data.flowRate.toFixed(2) + ' ml/s';
          } else {
            resultDiv.innerHTML = currentLang === 'ru' ? 'Нет данных калибровки.' : 'No calibration data yet.';
          }
        }
        if (calibrationInterval) {
          clearInterval(calibrationInterval);
          calibrationInterval = null;
        }
        // После остановки калибровки, если выбран режим fixed, показываем поле ввода объёма
        const type = document.querySelector('input[name="calibType"]:checked').value;
        if (type === 'fixed' && fixedVolumeBlock) {
          fixedVolumeBlock.style.display = 'block';
        }
        if (fixedProgress) fixedProgress.innerText = '';
      }
    })
    .catch(err => console.error('Failed to load calibration status', err));
}

function updateCalibrationTime() {
  fetchJSON('/calibrate/status')
    .then(data => {
      const statusDiv = document.getElementById('status');
      const toggleBtn = document.getElementById('toggleCalibBtn');
      const fixedProgress = document.getElementById('fixedProgress');
      if (data.calibrating) {
        if (statusDiv) statusDiv.innerText = translations[currentLang].calibratingStatus;
        if (toggleBtn) {
          toggleBtn.innerText = translations[currentLang].stopCalibration;
          toggleBtn.classList.remove('btn-off');
          toggleBtn.classList.add('btn-on');
        }
        if (data.fixed && data.remainingSec > 0 && fixedProgress) {
          fixedProgress.innerText = translations[currentLang].fixedRemaining.replace('{0}', data.remainingSec);
        }
      } else {
        clearInterval(calibrationInterval);
        calibrationInterval = null;
        if (toggleBtn) {
          toggleBtn.innerText = translations[currentLang].startCalibration;
          toggleBtn.classList.remove('btn-on');
          toggleBtn.classList.add('btn-off');
        }
        if (statusDiv) statusDiv.innerText = translations[currentLang].notCalibratingStatus;
        const resultDiv = document.getElementById('result');
        if (resultDiv && data.flowRate > 0) {
          resultDiv.innerHTML = (currentLang === 'ru' ? 'Калибровка: ' : 'Calibrated flow rate: ') + data.flowRate.toFixed(2) + ' ml/s';
        }
        // Если калибровка завершилась (в т.ч. по таймауту) и выбран fixed — показываем поле ввода
        const type = document.querySelector('input[name="calibType"]:checked').value;
        const fixedVolumeBlock = document.getElementById('fixedVolumeBlock');
        if (type === 'fixed' && fixedVolumeBlock) {
          fixedVolumeBlock.style.display = 'block';
        }
        if (fixedProgress) fixedProgress.innerText = '';
      }
    })
    .catch(err => console.error('Update failed', err));
}

function toggleCalibration() {
  const type = document.querySelector('input[name="calibType"]:checked').value;
  const statusDiv = document.getElementById('status');
  const isCalibrating = statusDiv && statusDiv.innerText.includes(translations[currentLang].calibratingStatus);
  
  if (isCalibrating) {
    stopCalibration();
  } else {
    if (type === 'fixed') {
      startFixedCalibration();
    } else if (type === 'manual') {
      alert('Для ручного ввода используйте кнопку "Сохранить"');
    } else {
      startCalibration();
    }
  }
}

function startCalibration() {
  const type = document.querySelector('input[name="calibType"]:checked').value;
  if (type !== 'standard' && type !== 'multi') {
    alert('Выберите обычную или многократную калибровку');
    return;
  }
  const volume = document.getElementById('volume').value;
  if (!volume || volume <= 0) {
    alert(translations[currentLang].targetVolume);
    return;
  }
  fetchJSON('/calibrate/start?volume=' + encodeURIComponent(volume), { method: 'POST' })
    .then(() => {
      loadCalibrationStatus();
    })
    .catch(err => {
      if (!err.message.includes('Redirecting')) {
        alert(translations[currentLang].error + err.message);
      }
    });
}

function stopCalibration() {
  fetch('/calibrate/stop', { method: 'POST' })
    .then(response => {
      if (response.ok) {
        const contentType = response.headers.get('content-type');
        if (contentType && contentType.includes('application/json')) {
          return response.json().then(data => {
            const type = document.querySelector('input[name="calibType"]:checked').value;
            if (type === 'multi') {
              if (calibrationResults.length >= 3) {
                alert(translations[currentLang].maxMeasurementsReached);
                return;
              }
              calibrationResults.push(data.flowRate);
              updateResultsList();
              if (calibrationResults.length === 3) {
                averageCalibrations();
                return;
              } else {
                loadCalibrationStatus();
              }
            } else {
              window.location.href = '/';
            }
          });
        } else {
          // Для фиксированной калибровки — просто обновить статус
          loadCalibrationStatus();
        }
      } else {
        return response.text().then(text => { throw new Error(text); });
      }
    })
    .catch(err => {
      if (!err.message.includes('Redirecting')) {
        alert(translations[currentLang].error + err.message);
      }
    });
}

function updateCalibrationType() {
  const type = document.querySelector('input[name="calibType"]:checked').value;
  document.getElementById('standardBlock').style.display = (type === 'standard' || type === 'multi') ? 'block' : 'none';
  document.getElementById('multiBlock').style.display = type === 'multi' ? 'block' : 'none';
  document.getElementById('fixedBlock').style.display = type === 'fixed' ? 'block' : 'none';
  document.getElementById('manualBlock').style.display = type === 'manual' ? 'block' : 'none';
  
  // Если выбран fixed и калибровка не активна, показываем поле ввода
  const fixedVolumeBlock = document.getElementById('fixedVolumeBlock');
  if (type === 'fixed' && fixedVolumeBlock) {
    const isCalibrating = document.getElementById('status').innerText.includes(translations[currentLang].calibratingStatus);
    if (!isCalibrating) {
      fixedVolumeBlock.style.display = 'block';
    } else {
      fixedVolumeBlock.style.display = 'none';
    }
  } else if (fixedVolumeBlock) {
    fixedVolumeBlock.style.display = 'none';
  }
}

document.querySelectorAll('input[name="calibType"]').forEach(radio => {
  radio.addEventListener('change', updateCalibrationType);
});

function updateResultsList() {
  const list = document.getElementById('resultsList');
  if (!list) return;
  list.innerHTML = '<h4 data-i18n="results">Результаты:</h4>';
  const ul = document.createElement('ul');
  calibrationResults.forEach((rate, index) => {
    const li = document.createElement('li');
    li.innerText = `${index+1}: ${rate.toFixed(3)} ml/s`;
    ul.appendChild(li);
  });
  list.appendChild(ul);
}

function averageCalibrations() {
  if (calibrationResults.length === 0) return;
  const n = calibrationResults.length;
  const sum = calibrationResults.reduce((a, b) => a + b, 0);
  const mean = sum / n;
  const variance = calibrationResults.reduce((a, b) => a + Math.pow(b - mean, 2), 0) / n;
  const stdDev = Math.sqrt(variance);
  const filtered = calibrationResults.filter(r => Math.abs(r - mean) <= 2 * stdDev);
  let finalMean;
  if (filtered.length === 0) {
    finalMean = mean;
  } else {
    finalMean = filtered.reduce((a, b) => a + b, 0) / filtered.length;
  }
  fetch('/calibrate/set', {
    method: 'POST',
    body: new URLSearchParams({ flowRate: finalMean })
  })
  .then(response => {
    if (!response.ok) throw new Error('HTTP error');
    alert(translations[currentLang].averageSaved + ' ' + finalMean.toFixed(3) + ' ml/s');
    calibrationResults = [];
    updateResultsList();
    loadCalibrationStatus();
  })
  .catch(err => alert(translations[currentLang].error + err.message));
}

function resetCalibrations() {
  calibrationResults = [];
  updateResultsList();
}

function startFixedCalibration() {
  const time = parseInt(document.getElementById('fixedTime').value);
  fetch('/calibrate/fixed/start?time=' + time, { method: 'POST' })
    .then(response => {
      if (response.ok) {
        // Скрываем поле ввода объёма и прогресс
        document.getElementById('fixedVolumeBlock').style.display = 'none';
        document.getElementById('fixedProgress').innerText = '';
        loadCalibrationStatus();
      } else {
        throw new Error('HTTP error');
      }
    })
    .catch(err => alert(translations[currentLang].error + err.message));
}

function saveFixedCalibration() {
  const volume = parseFloat(document.getElementById('fixedVolume').value);
  if (!volume || volume <= 0) {
    alert(translations[currentLang].fixedVolumeLabel);
    return;
  }
  const time = parseInt(document.getElementById('fixedTime').value);
  fetch('/calibrate/fixed/save', {
    method: 'POST',
    body: new URLSearchParams({ volume: volume, time: time })
  })
  .then(response => {
    if (response.ok) {
      alert(translations[currentLang].fixedSaved);
      document.getElementById('fixedVolume').value = '';
      document.getElementById('fixedVolumeBlock').style.display = 'none';
      loadCalibrationStatus();
    } else {
      throw new Error('HTTP error');
    }
  })
  .catch(err => alert(translations[currentLang].error + err.message));
}

function saveManualFlow() {
  const flow = parseFloat(document.getElementById('manualFlow').value);
  if (!flow || flow <= 0) {
    alert(translations[currentLang].manualFlowLabel);
    return;
  }
  fetch('/calibrate/manual', {
    method: 'POST',
    body: new URLSearchParams({ flowRate: flow })
  })
  .then(response => {
    if (response.ok) {
      alert(translations[currentLang].manualSaved);
      document.getElementById('manualFlow').value = '';
      loadCalibrationStatus();
    } else {
      throw new Error('HTTP error');
    }
  })
  .catch(err => alert(translations[currentLang].error + err.message));
}

// ============================================
// Экспорт/Импорт конфигурации
// ============================================

function exportConfig() {
  fetch('/api/export')
    .then(response => {
      if (!response.ok) throw new Error('Export failed');
      return response.blob();
    })
    .then(blob => {
      const url = window.URL.createObjectURL(blob);
      const a = document.createElement('a');
      a.href = url;
      a.download = 'afd_config.json';
      document.body.appendChild(a);
      a.click();
      window.URL.revokeObjectURL(url);
      document.body.removeChild(a);
      const resultDiv = document.getElementById('result');
      if (resultDiv) resultDiv.innerHTML = '<span style="color:#28a745;">' + translations[currentLang].exportSuccess + '</span>';
    })
    .catch(err => {
      const resultDiv = document.getElementById('result');
      if (resultDiv) resultDiv.innerHTML = '<span style="color:#dc3545;">' + translations[currentLang].error + err.message + '</span>';
    });
}

function importConfig(file) {
  if (!file) return;
  const reader = new FileReader();
  const resultDiv = document.getElementById('result');
  if (resultDiv) resultDiv.innerHTML = '<span style="color:#6c757d;">' + translations[currentLang].saving + ' <span class="loader"></span></span>';
  reader.onload = function(e) {
    const content = e.target.result;
    fetch('/api/import', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: content
    })
    .then(response => {
      if (!response.ok) throw new Error('Import failed');
      if (resultDiv) resultDiv.innerHTML = '<span style="color:#28a745;">' + translations[currentLang].importSuccess + '</span>';
      setTimeout(() => { window.location.href = '/'; }, 3000);
    })
    .catch(err => {
      if (resultDiv) resultDiv.innerHTML = '<span style="color:#dc3545;">' + translations[currentLang].importError + err.message + '</span>';
    });
  };
  reader.readAsText(file);
}

// ============================================
// Планировщик (Schedule)
// ============================================

let currentFlowRate = 0;

function formatDuration(seconds) {
  if (seconds < 60) {
    return seconds + ' с';
  } else {
    const mins = Math.floor(seconds / 60);
    const secs = seconds % 60;
    return mins + ' мин ' + secs + ' с';
  }
}

function loadJobs() {
  fetchJSON('/schedule/list')
    .then(data => {
      const tbody = document.querySelector('#jobTable tbody');
      if (!tbody) return;
      tbody.innerHTML = '';
      data.forEach(job => {
        const row = tbody.insertRow();
        row.insertCell().innerText = job.id;
        row.insertCell().innerText = job.hour.toString().padStart(2,'0') + ':' + job.minute.toString().padStart(2,'0');
        row.insertCell().innerText = job.volume;
        
        let durationText = '—';
        if (currentFlowRate > 0) {
          const durationSec = Math.round(job.volume / currentFlowRate);
          durationText = formatDuration(durationSec);
        }
        row.insertCell().innerText = durationText;
        
        const days = [];
        if (job.daysMask & 1) days.push(translations[currentLang].mon);
        if (job.daysMask & 2) days.push(translations[currentLang].tue);
        if (job.daysMask & 4) days.push(translations[currentLang].wed);
        if (job.daysMask & 8) days.push(translations[currentLang].thu);
        if (job.daysMask & 16) days.push(translations[currentLang].fri);
        if (job.daysMask & 32) days.push(translations[currentLang].sat);
        if (job.daysMask & 64) days.push(translations[currentLang].sun);
        row.insertCell().innerHTML = days.join(' ') || (currentLang === 'ru' ? 'Нет' : 'None');
        
        const enabledCell = row.insertCell();
        const enabledCheckbox = document.createElement('input');
        enabledCheckbox.type = 'checkbox';
        enabledCheckbox.checked = job.enabled;
        enabledCheckbox.dataset.id = job.id;
        enabledCheckbox.addEventListener('change', function(e) {
          toggleJob(job.id, this.checked);
        });
        enabledCell.appendChild(enabledCheckbox);
        
        const actions = row.insertCell();
        const delBtn = document.createElement('button');
        delBtn.innerText = translations[currentLang].actions === 'Действия' ? 'Удалить' : 'Delete';
        delBtn.className = 'btn btn-danger';
        delBtn.onclick = () => deleteJob(job.id);
        actions.appendChild(delBtn);
      });
    })
    .catch(err => console.error('Failed to load jobs', err));
}

function toggleJob(id, enabled) {
  const params = new URLSearchParams();
  params.append('id', id);
  params.append('enabled', enabled ? '1' : '0');

  fetch('/schedule/toggle', { method: 'POST', body: params })
    .then(response => {
      if (!response.ok) throw new Error('Failed to toggle job');
    })
    .catch(err => {
      alert(translations[currentLang].error + err.message);
      const checkbox = document.querySelector(`input[type="checkbox"][data-id="${id}"]`);
      if (checkbox) checkbox.checked = !enabled;
    });
}

function addJob() {
  if (!document.getElementById('jobTime')) return;

  const jobTime = document.getElementById('jobTime').value;
  if (!jobTime) {
    alert(translations[currentLang].jobTimeLabel + ' ' + translations[currentLang].error);
    return;
  }
  const [hour, minute] = jobTime.split(':').map(Number);

  let mask = 0;
  if (document.getElementById('mon').checked) mask |= 1;
  if (document.getElementById('tue').checked) mask |= 2;
  if (document.getElementById('wed').checked) mask |= 4;
  if (document.getElementById('thu').checked) mask |= 8;
  if (document.getElementById('fri').checked) mask |= 16;
  if (document.getElementById('sat').checked) mask |= 32;
  if (document.getElementById('sun').checked) mask |= 64;

  if (mask === 0) {
    alert(translations[currentLang].selectDays);
    return;
  }

  const volume = document.getElementById('volume').value;
  const enabled = document.getElementById('enabled').checked ? 1 : 0;

  if (!volume) {
    alert(translations[currentLang].volumeMl);
    return;
  }

  const params = new URLSearchParams();
  params.append('hour', hour);
  params.append('minute', minute);
  params.append('daysMask', mask);
  params.append('volume', volume);
  params.append('enabled', enabled);

  fetchJSON('/schedule/add', { method: 'POST', body: params })
    .then(() => {
      loadJobs();
      document.getElementById('jobTime').value = '';
      document.getElementById('volume').value = '';
      document.querySelectorAll('#addForm input[type=checkbox]').forEach(cb => cb.checked = false);
      document.getElementById('enabled').checked = true;
    })
    .catch(err => {
      if (!err.message.includes('Redirecting')) {
        alert(translations[currentLang].error + err.message);
      }
    });
}

function deleteJob(id) {
  if (!confirm(translations[currentLang].actions)) return;
  fetchJSON('/schedule/delete?id=' + id, { method: 'POST' })
    .then(() => loadJobs())
    .catch(err => {
      if (!err.message.includes('Redirecting')) {
        alert(translations[currentLang].error + err.message);
      }
    });
}

function updateMaxVolume(flowRate) {
  currentFlowRate = flowRate;
  const volumeWarning = document.getElementById('volumeWarning');
  if (volumeWarning) {
    volumeWarning.style.display = 'none';
  }
  loadJobs();
}

// ============================================
// Управление временем
// ============================================

function updateCurrentTime() {
  fetchJSON('/time/get')
    .then(data => {
      const el = document.getElementById('currentTime');
      if (el) {
        el.innerHTML = (currentLang === 'ru' ? 'Текущее: ' : 'Current: ') + data.date + ' ' + data.time;
      }
    })
    .catch(err => console.error(err));
}

function syncNTP() {
  fetchJSON('/time/ntp', { method: 'POST' })
    .then(() => {
      updateCurrentTime();
    })
    .catch(err => {
      if (!err.message.includes('Redirecting')) {
        alert(translations[currentLang].error + err.message);
      }
    });
}

function setTime() {
  const year = document.getElementById('year').value;
  const month = document.getElementById('month').value;
  const day = document.getElementById('day').value;
  const hour = document.getElementById('hour').value;
  const minute = document.getElementById('minute').value;
  const second = document.getElementById('second').value;

  if (!year || !month || !day || !hour || !minute || !second) {
    alert(translations[currentLang].manualSet);
    return;
  }

  const params = new URLSearchParams();
  params.append('year', year);
  params.append('month', month);
  params.append('day', day);
  params.append('hour', hour);
  params.append('minute', minute);
  params.append('second', second);

  fetchJSON('/time/set', { method: 'POST', body: params })
    .then(() => {
      updateCurrentTime();
    })
    .catch(err => {
      if (!err.message.includes('Redirecting')) {
        alert(translations[currentLang].error + err.message);
      }
    });
}

function loadNTPServer() {
  fetchJSON('/mqtt/get')
    .then(data => {
      const ntpServer = document.getElementById('ntpServer');
      const timezone = document.getElementById('timezone');
      if (ntpServer) ntpServer.value = data.ntpServer || 'pool.ntp.org';
      if (timezone) timezone.value = data.timezone || 3;
    })
    .catch(err => console.error('Failed to load NTP server', err));
}

function saveNTPServer() {
  const ntpServer = document.getElementById('ntpServer');
  const timezone = document.getElementById('timezone');
  if (!ntpServer || !timezone) return;

  const ntpServerVal = ntpServer.value;
  const timezoneVal = parseInt(timezone.value);

  if (!ntpServerVal) {
    alert(translations[currentLang].ntpServer);
    return;
  }

  fetchJSON('/mqtt/get')
    .then(data => {
      const params = new URLSearchParams();
      params.append('server', data.server);
      params.append('port', data.port);
      params.append('user', data.user);
      params.append('password', data.password);
      params.append('prefix', data.prefix);
      params.append('ntpServer', ntpServerVal);
      params.append('timezone', timezoneVal);
      return fetchJSON('/mqtt/save', { method: 'POST', body: params });
    })
    .then(() => {})
    .catch(err => {
      if (!err.message.includes('Redirecting')) {
        alert(translations[currentLang].error + err.message);
      }
    });
}

// ============================================
// MQTT страница
// ============================================

function loadMQTTConfig() {
  fetchJSON('/mqtt/get')
    .then(data => {
      document.getElementById('server').value = data.server;
      document.getElementById('port').value = data.port;
      document.getElementById('user').value = data.user;
      document.getElementById('password').value = data.password;
      document.getElementById('prefix').value = data.prefix;
    })
    .catch(err => console.error('Failed to load MQTT config', err));
}

function saveMQTT() {
  const server = document.getElementById('server').value;
  const port = document.getElementById('port').value;
  const user = document.getElementById('user').value;
  const password = document.getElementById('password').value;
  const prefix = document.getElementById('prefix').value;

  if (!server || !port || !prefix) {
    alert(translations[currentLang].mqttMissing);
    return;
  }

  const params = new URLSearchParams();
  params.append('server', server);
  params.append('port', port);
  params.append('user', user);
  params.append('password', password);
  params.append('prefix', prefix);

  const btn = document.getElementById('mqttSaveBtn');
  const originalText = btn.innerHTML;
  btn.disabled = true;
  btn.innerHTML = translations[currentLang].saving + ' <span class="loader"></span>';
  
  const resultDiv = document.getElementById('mqttResult');
  if (resultDiv) resultDiv.innerHTML = '';

  fetchJSON('/mqtt/save', { method: 'POST', body: params })
    .then(() => {
      if (resultDiv) resultDiv.innerHTML = '<span style="color:#28a745;">' + translations[currentLang].mqttSaveSuccess + '</span>';
      setTimeout(() => {
        window.location.href = '/';
      }, 3000);
    })
    .catch(err => {
      if (resultDiv) resultDiv.innerHTML = '<span style="color:#dc3545;">' + translations[currentLang].error + err.message + '</span>';
      btn.disabled = false;
      btn.innerHTML = originalText;
    });
}

function updateMQTTStatus() {
  fetchJSON('/mqtt/status')
    .then(data => {
      const el = document.getElementById('mqttStatus');
      if (el) {
        const isConnected = data.status === 'connected';
        el.innerText = (currentLang === 'ru' ? 'MQTT: ' : 'MQTT: ') + (isConnected 
          ? translations[currentLang].mqttConnected 
          : translations[currentLang].mqttDisconnected);
        el.style.color = isConnected ? '#28a745' : '#dc3545';
      }
    })
    .catch(err => console.error('MQTT status error', err));
}

function sendDiscovery() {
  const resultDiv = document.getElementById('discoveryResult');
  if (!resultDiv) return;
  resultDiv.innerHTML = 'Sending... <span class="loader"></span>';
  fetchJSON('/mqtt/discovery', { method: 'POST' })
    .then(() => {
      resultDiv.innerHTML = '<span style="color:#28a745;">' + (currentLang === 'ru' ? 'Discovery отправлен!' : 'Discovery sent successfully!') + '</span>';
    })
    .catch(err => {
      resultDiv.innerHTML = '<span style="color:#dc3545;">' + (currentLang === 'ru' ? 'Ошибка: ' : 'Error: ') + err.message + '</span>';
    });
}

// ============================================
// Инициализация по страницам
// ============================================

document.addEventListener('DOMContentLoaded', () => {
  const savedLang = localStorage.getItem('lang') || 'ru';
  applyLanguage(savedLang);
  
  document.querySelectorAll('.lang-btn').forEach(btn => {
    btn.addEventListener('click', () => {
      applyLanguage(btn.dataset.lang);
      const path = window.location.pathname;
      if (path === '/schedule') {
        loadJobs();
      }
      if (path === '/mqtt') loadMQTTConfig();
    });
  });

  const path = window.location.pathname;

  connectWebSocket();

  if (path === '/wifi') loadWiFiConfig();
  if (path === '/calibrate') {
    loadCalibrationStatus();
    updateCalibrationType();
    document.querySelectorAll('input[name="calibType"]').forEach(radio => {
      radio.addEventListener('change', updateCalibrationType);
    });
    setInterval(loadCalibrationStatus, 2000);
  }
  if (path === '/config') {
    loadBrightness();
    loadLevelConfig();
  }
  if (path === '/schedule') {
    loadJobs();
    fetchJSON('/calibration/status')
      .then(data => {
        if (!data.calibrated) {
          window.location.href = '/calibrate';
        } else {
          fetchJSON('/calibrate/status')
            .then(calData => {
              if (calData.flowRate) {
                updateMaxVolume(calData.flowRate);
              }
            })
            .catch(err => console.error('Failed to load flow rate', err));
        }
      })
      .catch(err => console.error('Failed to check calibration status', err));
  }
  if (path === '/time') {
    updateCurrentTime();
    setInterval(updateCurrentTime, 5000);
    loadNTPServer();
  }
  if (path === '/mqtt') {
    loadMQTTConfig();
    updateMQTTStatus();
    setInterval(updateMQTTStatus, 5000);
  }
});