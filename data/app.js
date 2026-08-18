let ws = null;
let spaState = {
  water_temp_f: 100,
  target_temp_f: 102,
  unit: 'F',
  pump1: 0,
  pump2: 0,
  blower: 0,
  blower_pct: 0,
  light: false,
  mode: 0,
  flow_ok: true,
  high_limit_ok: true,
  water_level_ok: true,
  heater: 0,
  jet_remaining: 0,
  blower_remaining: 0,
  status: 'Connecting...'
};

function initWebSocket() {
  const protocol = location.protocol === 'https:' ? 'wss:' : 'ws:';
  const wsUrl = protocol + '//' + location.host + '/ws';
  
  console.log('Connecting to WebSocket:', wsUrl);
  ws = new WebSocket(wsUrl);

  ws.onopen = () => {
    console.log('WebSocket Connected');
    document.getElementById('statusBadge').innerText = 'Connected';
    document.getElementById('statusBadge').style.background = 'rgba(46, 204, 113, 0.15)';
    document.getElementById('statusBadge').style.color = 'var(--accent-green)';
  };

  ws.onmessage = (event) => {
    try {
      spaState = JSON.parse(event.data);
      updateUI();
    } catch (e) {
      console.error('Error parsing JSON from server:', e);
    }
  };

  ws.onclose = () => {
    console.log('WebSocket Disconnected. Reconnecting in 2s...');
    document.getElementById('statusBadge').innerText = 'Reconnecting...';
    document.getElementById('statusBadge').style.background = 'rgba(231, 76, 60, 0.15)';
    document.getElementById('statusBadge').style.color = 'var(--accent-red)';
    setTimeout(initWebSocket, 2000);
  };

  ws.onerror = (err) => {
    console.error('WebSocket Error:', err);
    ws.close();
  };
}

function sendCommand(cmdObj) {
  if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify(cmdObj));
  } else {
    // Fallback to REST API
    fetch('/api/control', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(cmdObj)
    }).catch(console.error);
  }
}

function updateUI() {
  const isC = (spaState.unit === 'C');
  const curTemp = isC ? spaState.water_temp_c : spaState.water_temp_f;
  const tgtTemp = isC ? spaState.target_temp_c : spaState.target_temp_f;
  const unitSym = '&deg;' + (spaState.unit || 'F');

  // Temperature
  document.getElementById('waterTemp').innerHTML = curTemp.toFixed(1) + unitSym;
  document.getElementById('targetTemp').innerHTML = 'Target: ' + tgtTemp.toFixed(1) + unitSym;

  // Status Badge
  document.getElementById('statusBadge').innerText = spaState.status || 'Ready';

  // Jet 1
  const tJ1 = document.getElementById('tileJet1');
  const tJ1Text = document.getElementById('jet1StateText');
  const tJ1Timer = document.getElementById('jet1Timer');
  if (spaState.pump1 === 2) {
    tJ1.className = 'tile-btn active-blue';
    tJ1Text.innerText = 'HIGH';
  } else if (spaState.pump1 === 1) {
    tJ1.className = 'tile-btn active-green';
    tJ1Text.innerText = 'LOW (CIRC)';
  } else {
    tJ1.className = 'tile-btn';
    tJ1Text.innerText = 'OFF';
  }
  if (spaState.jet_remaining > 0) {
    const mins = Math.floor(spaState.jet_remaining / 60);
    const secs = spaState.jet_remaining % 60;
    tJ1Timer.innerText = mins + ':' + (secs < 10 ? '0' : '') + secs;
  } else {
    tJ1Timer.innerText = '--';
  }

  // Jet 2
  const tJ2 = document.getElementById('tileJet2');
  const tJ2Text = document.getElementById('jet2StateText');
  if (spaState.pump2 > 0) {
    tJ2.className = 'tile-btn active-blue';
    tJ2Text.innerText = 'ON';
  } else {
    tJ2.className = 'tile-btn';
    tJ2Text.innerText = 'OFF';
  }

  // Blower
  const tBlw = document.getElementById('tileBlower');
  const tBlwText = document.getElementById('blowerStateText');
  const tBlwTimer = document.getElementById('blowerTimer');
  if (spaState.blower > 0) {
    tBlw.className = 'tile-btn active-purple';
    tBlwText.innerText = (spaState.blower === 3) ? 'HIGH' : ((spaState.blower === 2) ? 'MED' : 'LOW');
  } else {
    tBlw.className = 'tile-btn';
    tBlwText.innerText = 'OFF';
  }
  if (spaState.blower_remaining > 0) {
    const mins = Math.floor(spaState.blower_remaining / 60);
    const secs = spaState.blower_remaining % 60;
    tBlwTimer.innerText = mins + ':' + (secs < 10 ? '0' : '') + secs;
  } else {
    tBlwTimer.innerText = '--';
  }

  // Blower Slider
  document.getElementById('sliderBlower').value = spaState.blower_pct || 0;
  document.getElementById('blowerPctLabel').innerText = (spaState.blower_pct || 0) + '%';

  // Light
  const tLgt = document.getElementById('tileLight');
  const tLgtText = document.getElementById('lightStateText');
  if (spaState.light) {
    tLgt.className = 'tile-btn active-orange';
    tLgtText.innerText = 'ON';
  } else {
    tLgt.className = 'tile-btn';
    tLgtText.innerText = 'OFF';
  }

  // Modes
  for (let i = 0; i < 4; i++) {
    const btn = document.getElementById('modeBtn' + i);
    if (btn) {
      btn.className = 'mode-btn' + (spaState.mode === i ? ' active' : '');
    }
  }

  // Diagnostics & Safety
  const elFlow = document.getElementById('valFlow');
  if (spaState.flow_ok) {
    elFlow.innerText = 'OK (Continuous)';
    elFlow.style.color = 'var(--accent-green)';
  } else {
    elFlow.innerText = 'FAULT (NO FLOW)';
    elFlow.style.color = 'var(--accent-red)';
  }

  const elHeater = document.getElementById('valHeater');
  if (spaState.heater === 2) {
    elHeater.innerText = 'HEATING (Element Active)';
    elHeater.style.color = 'var(--accent-orange)';
  } else if (spaState.heater === 1) {
    elHeater.innerText = 'VERIFYING PRE-FLOW (15s)';
    elHeater.style.color = '#f59e0b';
  } else if (spaState.heater === 3) {
    elHeater.innerText = 'POST-HEAT COOLDOWN';
    elHeater.style.color = '#38bdf8';
  } else {
    elHeater.innerText = 'OFF';
    elHeater.style.color = 'var(--text-muted)';
  }

  document.getElementById('valLimit').innerText = spaState.high_limit_ok ? 'OK (Intact)' : 'TRIPPED (Open)';
  document.getElementById('valLimit').style.color = spaState.high_limit_ok ? 'var(--accent-green)' : 'var(--accent-red)';
  document.getElementById('valLevel').innerText = spaState.water_level_ok ? 'OK' : 'LOW WATER';
  document.getElementById('valOzone').innerText = spaState.ozone ? 'ACTIVE' : 'OFF';
  document.getElementById('valTime').innerText = (spaState.time || '--:--') + ' (' + (spaState.date || '') + ')';
}

function adjustTemp(delta) {
  const current = spaState.target_temp_f || 100;
  const newTemp = Math.min(104, Math.max(80, current + delta));
  spaState.target_temp_f = newTemp;
  updateUI();
  sendCommand({ action: 'set_temp', temp: newTemp });
}

function toggleJet1() {
  let next = 0;
  if (spaState.pump1 === 0) next = 1;
  else if (spaState.pump1 === 1) next = 2;
  else next = 0;
  spaState.pump1 = next;
  updateUI();
  sendCommand({ action: 'set_jet1', speed: next });
}

function toggleJet2() {
  const next = (spaState.pump2 > 0) ? 0 : 2;
  spaState.pump2 = next;
  updateUI();
  sendCommand({ action: 'set_jet2', speed: next });
}

function toggleBlower() {
  let next = 0;
  if (spaState.blower === 0) next = 1;
  else if (spaState.blower === 1) next = 2;
  else if (spaState.blower === 2) next = 3;
  else next = 0;
  spaState.blower = next;
  updateUI();
  sendCommand({ action: 'set_blower', speed: next });
}

function onBlowerSlider(val) {
  document.getElementById('blowerPctLabel').innerText = val + '%';
  sendCommand({ action: 'set_blower_pct', pct: parseInt(val) });
}

function toggleLight() {
  spaState.light = !spaState.light;
  updateUI();
  sendCommand({ action: 'set_light', state: spaState.light });
}

function setMode(m) {
  spaState.mode = m;
  updateUI();
  sendCommand({ action: 'set_mode', mode: m });
}

window.addEventListener('DOMContentLoaded', initWebSocket);
