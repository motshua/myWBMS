const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>myWBMS Dashboard</title>
  <meta name="viewport" content="width=device-width, initial-scale=1" charset="UTF-8">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <link rel="icon" href="data:,">
  <style>
    html { font-family: Arial; display: inline-block; text-align: center; }
    h4 { text-align: left; margin-left: 5%; margin-bottom: 0; }
    p { font-size: 1.2rem; }
    p.stateOfCharge { font-size: 2rem; }
    p.info { font-size: 0.9rem; }
    p.config { font-size: 0.9rem; }
    input { width: 100%; text-align: right; }
    hr { border-top: 1px solid; color: #bebebe; width: 90%; }
    body { margin: 0; }
    .topnav { overflow: hidden; background-color: #2f4468; color: white; font-size: 1.7rem; }
    .content { padding: 20px; }
    .cards { max-width: 700px; margin: 0 auto; display: grid; grid-gap: 2rem; grid-template-columns: 1fr; }
    .card { background-color: white; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5); }
    .cardPack { display: grid; grid-gap: 0; grid-template-columns: 1fr 1fr 1fr; justify-content: center; align-items: center; margin-left: 5%; margin-right: 5%; min-width: 120px; }
    .reading { font-size: 2.8rem; }
    .packet { color: #bebebe; margin-top: -20px; font-size: 0.9rem; }
    .temperature { color: #fd7e14; }
    .humidity { color: #1b78e2; }
    .batteryContainerCell { display: -webkit-box; display: -moz-box; display: -ms-flexbox; display: -webkit-flex; display: inline-flex; flex-direction: row; align-items: center; justify-content: left; margin-left: 2px; }
    .batteryOuterCell { border-radius: 3px; border: 1px solid #444; padding: 1px; width: 35px; height: 9px; }
    .batteryBumpCell { border-radius: 2px; background-color: #444; margin: 2px; width: 1px; height: 3px; }
    .batteryLevelCell { border-radius: 2px; background-color: #73AD21; height: 9px; }
    .btn { -webkit-border-radius: 2px; -moz-border-radius: 2px; border-radius: 2px; background-color: white; padding: 2px 7px 2px 7px; border: 0.5px solid #000; color: black; width: 100%; border-color: rgb(118, 118, 118); cursor: pointer; }
  </style>
</head>
<body>
  <div class="topnav">
    <h3><span id="nav_title_txt"></span></h3>
  </div>
  <div class="content">
    <div class="cards">

      <div class="card">
        <h4>BMU 
          <span style="float: right; margin-right: 5%;">
            <i class="fas fa-info-circle" onclick="showPackInfo()"></i> 
            <i class="fas fa-cog" onclick="showPackConfig()"></i>
          </span>
        </h4>
        <hr>

        <div class="cardPack">
          <div>
            <p><span id="pack_voltage"></span> V</p>
            <p class="packet" id="pack_voltage_txt"></p>
            <p><span id="avg_cell_voltage"></span> V</p>
            <p class="packet" id="avg_cell_voltage_txt"></p>
          </div>
          <div>
            <p class="stateOfCharge"><span id="pack_sob_perc"></span>%</p>
            <p class="packet" style="margin-top: -35px; margin-bottom: 35px;" id="pack_sob_txt"></p>
          </div>
          <div>
            <p><span id="load_current"></span> A</p>
            <p class="packet" id="load_current_txt"></p>
            <p><span id="charging_current"></span> A</p>
            <p class="packet" id="charging_current_txt"></p>
          </div>
        </div>
        <p></p>
        <p class="packet"><span id="last_msg_bmu_txt"></span><span id="last_msg_bmu"></span></p>

        <div id="packInfo" style="display: none;">
          <hr>
          <p class="info"><span id="pack_charge_capacity_txt"></span><span id="pack_charge_capacity"></span> mAh</p>
          <p class="info"><span id="pack_state_of_charge_txt"></span><span id="pack_state_of_charge"></span> mAh (<span id="pack_state_of_charge_perc"></span>%)</p>
          <p class="info"><span id="pack_energy_capacity_txt"></span><span id="pack_energy_capacity"></span> mWh</p>
          <p class="info"><span id="pack_state_of_energy_txt"></span><span id="pack_state_of_energy"></span> mWh (<span id="pack_state_of_energy_perc"></span>%)</p>
          <p class="info"><span id="pack_voltage_BMU_txt"></span><span id="pack_voltage_BMU"></span> V</p>
          <p class="info"><span id="charging_voltage_txt"></span><span id="charging_voltage"></span> V</p>
          <p class="info"><span id="pack_voltage_BMM_txt"></span><span id="pack_voltage_BMM"></span> V</p>
          <p class="info"><span id="min_cell_voltage_txt"></span><span id="min_cell_voltage"></span> V</p>
          <p class="info"><span id="min_cell_voltage_release_txt"></span><span id="min_cell_voltage_release"></span> V</p>
          <p class="info"><span id="max_cell_voltage_txt"></span><span id="max_cell_voltage"></span> V</p>
          <p class="info"><span id="max_cell_voltage_release_txt"></span><span id="max_cell_voltage_release"></span> V</p>
          <p class="info"><span id="max_load_current_txt"></span><span id="max_load_current"></span> A</p>
          <p class="info"><span id="max_charging_current_txt"></span><span id="max_charging_current"></span> A</p>
          <p class="info"><span id="min_cell_temp_txt"></span><span id="min_cell_temp"></span> &deg;C</p>
          <p class="info"><span id="max_cell_temp_txt"></span><span id="max_cell_temp"></span> &deg;C</p>
          <p class="info"><span id="max_cell_bal_temp_txt"></span><span id="max_cell_bal_temp"></span> &deg;C</p>
          <p class="info"><span id="series_cell_num_txt"></span><span id="series_cell_num"></span></p>
          <p class="info"><span id="highest_cell_voltage_txt"></span> (#<span id="highest_cell_voltage_index"></span>) <span id="highest_cell_voltage"></span> V</p>
          <p class="info"><span id="lowest_cell_voltage_txt"></span> (#<span id="lowest_cell_voltage_index"></span>) <span id="lowest_cell_voltage"></span> V</p>
          <p class="info"><span id="charging_switch_state_txt"></span><span id="charging_switch_state"></span></p>
          <p class="info"><span id="discharging_switch_state_txt"></span><span id="discharging_switch_state"></span></p>
          <p class="info"><span id="error_code_bmu_txt"></span><span id="error_code_bmu"></span></p>
          <p class="info">MAC: <span id="mac_bmu"></span></p>
        </div>

        <div id="packConfig" style="display: none;">
          <hr>
          <form method="get" action="/get" style="margin: 0 4% 0 4%;">
            <div style="padding: 0 20px;">
              <p></p>
              <select name="curr_lang" id="curr_lang" style="display: none;">
                <option value="null"></option>
                <option value="pt_br"></option>
                <option value="en_us"></option>
              </select>
              <div style="display: flex; align-items: center; margin-bottom: 10px;">
                <p class="config" style="margin: 0 10px 0 0; white-space: pre;" id="min_cell_voltage_config_txt"></p>
                <div style="flex-grow: 1">
                  <input type="number" step="0.01" value="" id="min_cell_voltage_config" name="min_cell_voltage_config">
                </div>
                <p class="config" style="margin: 0 0 0 15px;">V</p>
              </div>
              <div style="display: flex; align-items: center; margin-bottom: 10px;">
                <p class="config" style="margin: 0 10px 0 0; white-space: pre;" id="min_cell_voltage_release_config_txt"></p>
                <div style="flex-grow: 1">
                  <input type="number" step="0.01" value="" id="min_cell_voltage_release_config" name="min_cell_voltage_release_config">
                </div>
                <p class="config" style="margin: 0 0 0 15px;">V</p>
              </div>
              <div style="display: flex; align-items: center; margin-bottom: 10px;">
                <p class="config" style="margin: 0 10px 0 0; white-space: pre;" id="max_cell_voltage_config_txt"></p>
                <div style="flex-grow: 1">
                  <input type="number" step="0.01" value="" id="max_cell_voltage_config" name="max_cell_voltage_config">
                </div>
                <p class="config" style="margin: 0 0 0 15px;">V</p>
              </div>
              <div style="display: flex; align-items: center; margin-bottom: 10px;">
                <p class="config" style="margin: 0 10px 0 0; white-space: pre;" id="max_cell_voltage_release_config_txt"></p>
                <div style="flex-grow: 1">
                  <input type="number" step="0.01" value="" id="max_cell_voltage_release_config" name="max_cell_voltage_release_config">
                </div>
                <p class="config" style="margin: 0 0 0 15px;">V</p>
              </div>
              <div style="display: flex; align-items: center; margin-bottom: 10px;">
                <p class="config" style="margin: 0 10px 0 0; white-space: pre;" id="max_load_current_config_txt"></p>
                <div style="flex-grow: 1">
                  <input type="number" step="0.01" value="" id="max_load_current_config" name="max_load_current_config">
                </div>
                <p class="config" style="margin: 0 0 0 15px;">A</p>
              </div>
              <div style="display: flex; align-items: center; margin-bottom: 10px;">
                <p class="config" style="margin: 0 10px 0 0; white-space: pre;" id="max_charging_current_config_txt"></p>
                <div style="flex-grow: 1">
                  <input type="number" step="0.01" value="" id="max_charging_current_config" name="max_charging_current_config">
                </div>
                <p class="config" style="margin: 0 0 0 15px;">A</p>
              </div>
              <div style="display: flex; align-items: center; margin-bottom: 10px;">
                <p class="config" style="margin: 0 10px 0 0; white-space: pre;" id="charge_capacity_config_txt"></p>
                <div style="flex-grow: 1">
                  <input type="number" step="0.01" value="" id="charge_capacity_config" name="charge_capacity_config">
                </div>
                <p class="config" style="margin: 0 0 0 15px;">mAh</p>
              </div>
              <div style="display: flex; align-items: center; margin-bottom: 10px;">
                <p class="config" style="margin: 0 10px 0 0; white-space: pre;" id="energy_capacity_config_txt"></p>
                <div style="flex-grow: 1">
                  <input type="number" step="0.01" value="" id="energy_capacity_config" name="energy_capacity_config">
                </div>
                <p class="config" style="margin: 0 0 0 15px;">mWh</p>
              </div>
              <div style="display: flex; align-items: center; margin-bottom: 10px;">
                <p class="config" style="margin: 0 10px 0 0; white-space: pre;" id="min_cell_temp_config_txt"></p>
                <div style="flex-grow: 1">
                  <input type="number" step="0.01" value="" id="min_cell_temp_config" name="min_cell_temp_config">
                </div>
                <p class="config" style="margin: 0 0 0 15px;">&deg;C</p>
              </div>
              <div style="display: flex; align-items: center; margin-bottom: 10px;">
                <p class="config" style="margin: 0 10px 0 0; white-space: pre;" id="max_cell_temp_config_txt"></p>
                <div style="flex-grow: 1">
                  <input type="number" step="0.01" value="" id="max_cell_temp_config" name="max_cell_temp_config">
                </div>
                <p class="config" style="margin: 0 0 0 15px;">&deg;C</p>
              </div>
              <div style="display: flex; align-items: center; margin-bottom: 10px;">
                <p class="config" style="margin: 0 10px 0 0; white-space: pre;" id="max_cell_bal_temp_config_txt"></p>
                <div style="flex-grow: 1">
                  <input type="number" step="0.01" value="" id="max_cell_bal_temp_config" name="max_cell_bal_temp_config">
                </div>
                <p class="config" style="margin: 0 0 0 15px;">&deg;C</p>
              </div>
              <div style="display: flex; align-items: center; margin-bottom: 10px;">
                <p class="config" style="margin: 0 10px 0 0; white-space: pre;" id="eject_sd_card_config_txt"></p>
                <div style="flex-grow: 1">
                  <select name="eject_sd_card_config" id="eject_sd_card_config" style="display: none;">
                    <option value="no"></option>
                    <option value="yes"></option>
                  </select>
                  <input type="button" id="eject_sd_card_config_btn" onclick="eject_sd_card_config_btn_func()" class="btn" value="">
                </div>
              </div>
              <div style="display: flex; align-items: center; margin-bottom: 10px;">
                <p class="config" style="margin: 0 10px 0 0; white-space: pre;" id="lang_config_txt"></p>
                <div style="flex-grow: 1">
                  <select name="lang_config" id="lang_config" style="display: none;">
                    <option value="pt_br"></option>
                    <option value="en_us"></option>
                  </select>
                  <input type="button" id="lang_config_btn" onclick="lang_config_btn_func()" class="btn" value="">
                </div>
              </div>
              <div style="display: flex; align-items: center; margin-bottom: 10px;">
                <input type="submit" value="" id="submit_txt" onclick="submitMessage()" style="margin: 0 0 0 0; text-align: center;">
              </div>
              <p></p>
            </div>
          </form>
        </div>
      </div>
      
      <div class="card">
        <h4>BMM #0 <span style="float: right; margin-right: 5%;"><i class="fas fa-info-circle" onclick="showModule1Info()"></i></span></h4>
        <hr>
        <p></p>
        <span id="cell_1_txt"></span>&nbsp;&nbsp;
        <span>&nbsp;<span id="cell_1_voltage"></span> V</span>
        <span>&nbsp;&nbsp;<span id="cell_1_temp"></span> &deg;C</span>
        <p></p>
        <span id="cell_2_txt"></span>&nbsp;&nbsp;
        <span>&nbsp;<span id="cell_2_voltage"></span> V</span>
        <span>&nbsp;&nbsp;<span id="cell_2_temp"></span> &deg;C</span>
        <p></p>
        <span id="cell_3_txt"></span>&nbsp;&nbsp;
        <span>&nbsp;<span id="cell_3_voltage"></span> V</span>
        <span>&nbsp;&nbsp;<span id="cell_3_temp"></span> &deg;C</span>
        <p></p>
        <p class="packet" style="margin-top: 10px;"><span id="last_msg_bmm_0_txt"></span><span id="last_msg_bmm_0"></span></p>
        <div id="module1Info" style="display: none;">
          <hr>
          <p class="info"><span id="cell_1_info_txt"></span> &nbsp;&nbsp;&nbsp;<span id="cell_1_bal_state"></span> &nbsp;&nbsp;&nbsp;<span id="cell_1_bal_temp"></span> &deg;C</p>
          <p class="info"><span id="cell_2_info_txt"></span> &nbsp;&nbsp;&nbsp;<span id="cell_2_bal_state"></span> &nbsp;&nbsp;&nbsp;<span id="cell_2_bal_temp"></span> &deg;C</p>
          <p class="info"><span id="cell_3_info_txt"></span> &nbsp;&nbsp;&nbsp;<span id="cell_3_bal_state"></span> &nbsp;&nbsp;&nbsp;<span id="cell_3_bal_temp"></span> &deg;C</p>
          <p class="info"><span id="time_awake_bmm_0_txt"></span><span id="time_awake_bmm_0"></span> ms</p>
          <p class="info"><span id="time_waiting_bmm_0_txt"></span><span id="time_waiting_bmm_0"></span> ms</p>
          <p class="info"><span id="error_code_bmm_0_txt"></span><span id="error_code_bmm_0"></span></p>
          <p class="info">MAC: <span id="mac_bmm_0"></span></p>
        </div>
      </div>
      
      <div class="card">
        <h4>BMM #1 <span style="float: right; margin-right: 5%;"><i class="fas fa-info-circle" onclick="showModule2Info()"></i></span></h4>
        <hr>
        <p></p>
        <span id="cell_4_txt"></span>&nbsp;&nbsp;
        <span>&nbsp;<span id="cell_4_voltage"></span> V</span>
        <span>&nbsp;&nbsp;<span id="cell_4_temp"></span> &deg;C</span>
        <p></p>
        <span id="cell_5_txt"></span>&nbsp;&nbsp;
        <span>&nbsp;<span id="cell_5_voltage"></span> V</span>
        <span>&nbsp;&nbsp;<span id="cell_5_temp"></span> &deg;C</span>
        <p></p>
        <span id="cell_6_txt"></span>&nbsp;&nbsp;
        <span>&nbsp;<span id="cell_6_voltage"></span> V</span>
        <span>&nbsp;&nbsp;<span id="cell_6_temp"></span> &deg;C</span>
        <p></p>
        <p class="packet" style="margin-top: 10px;"><span id="last_msg_bmm_1_txt"></span><span id="last_msg_bmm_1"></span></p>
        <div id="module2Info" style="display: none;">
          <hr>
          <p class="info"><span id="cell_4_info_txt"></span> &nbsp;&nbsp;&nbsp;<span id="cell_4_bal_state"></span> &nbsp;&nbsp;&nbsp;<span id="cell_4_bal_temp"></span> &deg;C</p>
          <p class="info"><span id="cell_5_info_txt"></span> &nbsp;&nbsp;&nbsp;<span id="cell_5_bal_state"></span> &nbsp;&nbsp;&nbsp;<span id="cell_5_bal_temp"></span> &deg;C</p>
          <p class="info"><span id="cell_6_info_txt"></span> &nbsp;&nbsp;&nbsp;<span id="cell_6_bal_state"></span> &nbsp;&nbsp;&nbsp;<span id="cell_6_bal_temp"></span> &deg;C</p>
          <p class="info"><span id="time_awake_bmm_1_txt"></span><span id="time_awake_bmm_1"></span> ms</p>
          <p class="info"><span id="time_waiting_bmm_1_txt"></span><span id="time_waiting_bmm_1"></span> ms</p>
          <p class="info"><span id="error_code_bmm_1_txt"></span><span id="error_code_bmm_1"></span></p>
          <p class="info">MAC: <span id="mac_bmm_1"></span></p>
        </div>
      </div>
      
      <div class="card">
        <h4>BMM #2 <span style="float: right; margin-right: 5%;"><i class="fas fa-info-circle" onclick="showModule3Info()"></i></span></h4>
        <hr>
        <p></p>
        <span id="cell_7_txt"></span>&nbsp;&nbsp;
        <span>&nbsp;<span id="cell_7_voltage"></span> V</span>
        <span>&nbsp;&nbsp;<span id="cell_7_temp"></span> &deg;C</span>
        <p></p>
        <span id="cell_8_txt"></span>&nbsp;&nbsp;
        <span>&nbsp;<span id="cell_8_voltage"></span> V</span>
        <span>&nbsp;&nbsp;<span id="cell_8_temp"></span> &deg;C</span>
        <p></p>
        <span id="cell_9_txt"></span>&nbsp;&nbsp;
        <span>&nbsp;<span id="cell_9_voltage"></span> V</span>
        <span>&nbsp;&nbsp;<span id="cell_9_temp"></span> &deg;C</span>
        <p></p>
        <p class="packet" style="margin-top: 10px;"><span id="last_msg_bmm_2_txt"></span><span id="last_msg_bmm_2"></span></p>
        <div id="module3Info" style="display: none;">
          <hr>
          <p class="info"><span id="cell_7_info_txt"></span> &nbsp;&nbsp;&nbsp;<span id="cell_7_bal_state"></span> &nbsp;&nbsp;&nbsp;<span id="cell_7_bal_temp"></span> &deg;C</p>
          <p class="info"><span id="cell_8_info_txt"></span> &nbsp;&nbsp;&nbsp;<span id="cell_8_bal_state"></span> &nbsp;&nbsp;&nbsp;<span id="cell_8_bal_temp"></span> &deg;C</p>
          <p class="info"><span id="cell_9_info_txt"></span> &nbsp;&nbsp;&nbsp;<span id="cell_9_bal_state"></span> &nbsp;&nbsp;&nbsp;<span id="cell_9_bal_temp"></span> &deg;C</p>
          <p class="info"><span id="time_awake_bmm_2_txt"></span><span id="time_awake_bmm_2"></span> ms</p>
          <p class="info"><span id="time_waiting_bmm_2_txt"></span><span id="time_waiting_bmm_2"></span> ms</p>
          <p class="info"><span id="error_code_bmm_2_txt"></span><span id="error_code_bmm_2"></span></p>
          <p class="info">MAC: <span id="mac_bmm_2"></span></p>
        </div>
      </div>

    </div>
  </div>
<script>

var sd_btn_updated = 0;

if (!!window.EventSource) {
  var source = new EventSource('/events');
 
  source.addEventListener('open', function(e) {
    console.log("Events Connected");
  }, false);

  source.addEventListener('error', function(e) {
    if (e.target.readyState != EventSource.OPEN) {
      console.log("Events Disconnected");
    }
  }, false);
 
  source.addEventListener('message', function(e) {
    console.log("message", e.data);
  }, false);

  source.addEventListener('lang', function(e) {
    //console.log("lang", e.data);
    var obj = JSON.parse(e.data);
    if (document.getElementById("curr_lang").value != obj.lang) {
      console.log("Changing language to " + obj.lang + "...");
      changeLang(obj.lang);
    }
  }, false);
 
  source.addEventListener('new_bmu', function(e) {
    //console.log("new_bmu", e.data);
    var obj = JSON.parse(e.data);
    document.getElementById("pack_voltage").innerHTML = obj.pack_voltage.toFixed(2);
    document.getElementById("avg_cell_voltage").innerHTML = obj.avg_cell_voltage.toFixed(2);
    document.getElementById("load_current").innerHTML = obj.load_current.toFixed(3);
    document.getElementById("charging_current").innerHTML = obj.charging_current.toFixed(3);
    document.getElementById("last_msg_bmu").innerHTML = obj.last_msg_bmu;
    document.getElementById("pack_sob_perc").innerHTML = obj.pack_state_of_energy_perc.toFixed(2);
    document.getElementById("pack_state_of_charge").innerHTML = obj.pack_state_of_charge.toFixed(0);
    document.getElementById("pack_state_of_charge_perc").innerHTML = obj.pack_state_of_charge_perc.toFixed(2);
    document.getElementById("pack_charge_capacity").innerHTML = obj.pack_charge_capacity.toFixed(0);
    document.getElementById("pack_state_of_energy").innerHTML = obj.pack_state_of_energy.toFixed(0);
    document.getElementById("pack_state_of_energy_perc").innerHTML = obj.pack_state_of_energy_perc.toFixed(2);
    document.getElementById("pack_energy_capacity").innerHTML = obj.pack_energy_capacity.toFixed(0);
    document.getElementById("pack_voltage_BMU").innerHTML = obj.pack_voltage_BMU.toFixed(2);
    document.getElementById("charging_voltage").innerHTML = obj.charging_voltage.toFixed(2);
    document.getElementById("pack_voltage_BMM").innerHTML = obj.pack_voltage.toFixed(2);
    document.getElementById("min_cell_voltage").innerHTML = obj.min_cell_voltage.toFixed(2);
    document.getElementById("min_cell_voltage_release").innerHTML = obj.min_cell_voltage_release.toFixed(2);
    document.getElementById("max_cell_voltage").innerHTML = obj.max_cell_voltage.toFixed(2);
    document.getElementById("max_cell_voltage_release").innerHTML = obj.max_cell_voltage_release.toFixed(2);
    document.getElementById("max_load_current").innerHTML = obj.max_load_current.toFixed(2);
    document.getElementById("max_charging_current").innerHTML = obj.max_charging_current.toFixed(2);
    document.getElementById("min_cell_temp").innerHTML = obj.min_cell_temp.toFixed(2);
    document.getElementById("max_cell_temp").innerHTML = obj.max_cell_temp.toFixed(2);
    document.getElementById("max_cell_bal_temp").innerHTML = obj.max_cell_bal_temp.toFixed(2);
    document.getElementById("series_cell_num").innerHTML = obj.series_cell_num;
    document.getElementById("lowest_cell_voltage").innerHTML = obj.lowest_cell_voltage.toFixed(2);
    document.getElementById("lowest_cell_voltage_index").innerHTML = obj.lowest_cell_voltage_index;
    document.getElementById("highest_cell_voltage").innerHTML = obj.highest_cell_voltage.toFixed(2);
    document.getElementById("highest_cell_voltage_index").innerHTML = obj.highest_cell_voltage_index;
    document.getElementById("charging_switch_state").innerHTML = obj.charging_switch_state;
    document.getElementById("discharging_switch_state").innerHTML = obj.discharging_switch_state;
    document.getElementById("error_code_bmu").innerHTML = obj.error_code_bmu;
    document.getElementById("mac_bmu").innerHTML = obj.mac_bmu;
    if (document.getElementById("min_cell_voltage_config").value == "")
      document.getElementById("min_cell_voltage_config").value = obj.min_cell_voltage.toFixed(2);
    if (document.getElementById("min_cell_voltage_release_config").value == "")
      document.getElementById("min_cell_voltage_release_config").value = obj.min_cell_voltage_release.toFixed(2);
    if (document.getElementById("max_cell_voltage_config").value == "")
      document.getElementById("max_cell_voltage_config").value = obj.max_cell_voltage.toFixed(2);
    if (document.getElementById("max_cell_voltage_release_config").value == "")
      document.getElementById("max_cell_voltage_release_config").value = obj.max_cell_voltage_release.toFixed(2);
    if (document.getElementById("max_load_current_config").value == "")
      document.getElementById("max_load_current_config").value = obj.max_load_current.toFixed(2);
    if (document.getElementById("max_charging_current_config").value == "")
      document.getElementById("max_charging_current_config").value = obj.max_charging_current.toFixed(2);
    if (document.getElementById("charge_capacity_config").value == "")
      document.getElementById("charge_capacity_config").value = obj.pack_charge_capacity.toFixed(2);
    if (document.getElementById("energy_capacity_config").value == "")
      document.getElementById("energy_capacity_config").value = obj.pack_energy_capacity.toFixed(2);
    if (document.getElementById("min_cell_temp_config").value == "")
      document.getElementById("min_cell_temp_config").value = obj.min_cell_temp.toFixed(2);
    if (document.getElementById("max_cell_temp_config").value == "")
      document.getElementById("max_cell_temp_config").value = obj.max_cell_temp.toFixed(2);
    if (document.getElementById("max_cell_bal_temp_config").value == "")
      document.getElementById("max_cell_bal_temp_config").value = obj.max_cell_bal_temp.toFixed(2);

    if ( (document.getElementById("eject_sd_card_config").value != obj.eject_sd_card) && (!sd_btn_updated) ) {
      document.getElementById("eject_sd_card_config").value = obj.eject_sd_card;
      update_lang_of_btns();
      sd_btn_updated = 1;
    }

  }, false);

  source.addEventListener('new_bmm_0', function(e) {
    //console.log("new_bmm_0", e.data);
    var obj = JSON.parse(e.data);
    document.getElementById("error_code_bmm_0").innerHTML = obj.error_code;
    document.getElementById("cell_1_voltage").innerHTML = obj.cell_1_voltage.toFixed(2);
    document.getElementById("cell_1_temp").innerHTML = obj.cell_1_temp.toFixed(2);
    document.getElementById("cell_1_bal_state").innerHTML = obj.cell_1_bal_state;
    document.getElementById("cell_1_bal_temp").innerHTML = obj.cell_1_bal_temp.toFixed(2);
    document.getElementById("cell_2_voltage").innerHTML = obj.cell_2_voltage.toFixed(2);
    document.getElementById("cell_2_temp").innerHTML = obj.cell_2_temp.toFixed(2);
    document.getElementById("cell_2_bal_state").innerHTML = obj.cell_2_bal_state;
    document.getElementById("cell_2_bal_temp").innerHTML = obj.cell_2_bal_temp.toFixed(2);
    document.getElementById("cell_3_voltage").innerHTML = obj.cell_3_voltage.toFixed(2);
    document.getElementById("cell_3_temp").innerHTML = obj.cell_3_temp.toFixed(2);
    document.getElementById("cell_3_bal_state").innerHTML = obj.cell_3_bal_state;
    document.getElementById("cell_3_bal_temp").innerHTML = obj.cell_3_bal_temp.toFixed(2);
    document.getElementById("time_awake_bmm_0").innerHTML = obj.time_awake;
    document.getElementById("time_waiting_bmm_0").innerHTML = obj.time_waiting;
    document.getElementById("mac_bmm_0").innerHTML = obj.mac_bmm;
    document.getElementById("last_msg_bmm_0").innerHTML = obj.last_msg;
  }, false);

  source.addEventListener('new_bmm_1', function(e) {
    //console.log("new_bmm_1", e.data);
    var obj = JSON.parse(e.data);
    document.getElementById("error_code_bmm_1").innerHTML = obj.error_code;
    document.getElementById("cell_4_voltage").innerHTML = obj.cell_1_voltage.toFixed(2);;
    document.getElementById("cell_4_temp").innerHTML = obj.cell_1_temp.toFixed(2);;
    document.getElementById("cell_4_bal_state").innerHTML = obj.cell_1_bal_state;
    document.getElementById("cell_4_bal_temp").innerHTML = obj.cell_1_bal_temp.toFixed(2);;
    document.getElementById("cell_5_voltage").innerHTML = obj.cell_2_voltage.toFixed(2);;
    document.getElementById("cell_5_temp").innerHTML = obj.cell_2_temp.toFixed(2);;
    document.getElementById("cell_5_bal_state").innerHTML = obj.cell_2_bal_state;
    document.getElementById("cell_5_bal_temp").innerHTML = obj.cell_2_bal_temp.toFixed(2);;
    document.getElementById("cell_6_voltage").innerHTML = obj.cell_3_voltage.toFixed(2);;
    document.getElementById("cell_6_temp").innerHTML = obj.cell_3_temp.toFixed(2);;
    document.getElementById("cell_6_bal_state").innerHTML = obj.cell_3_bal_state;
    document.getElementById("cell_6_bal_temp").innerHTML = obj.cell_3_bal_temp.toFixed(2);;
    document.getElementById("time_awake_bmm_1").innerHTML = obj.time_awake;
    document.getElementById("time_waiting_bmm_1").innerHTML = obj.time_waiting;
    document.getElementById("mac_bmm_1").innerHTML = obj.mac_bmm;
    document.getElementById("last_msg_bmm_1").innerHTML = obj.last_msg;
  }, false);

  source.addEventListener('new_bmm_2', function(e) {
    //console.log("new_bmm_2", e.data);
    var obj = JSON.parse(e.data);
    document.getElementById("error_code_bmm_2").innerHTML = obj.error_code;
    document.getElementById("cell_7_voltage").innerHTML = obj.cell_1_voltage.toFixed(2);;
    document.getElementById("cell_7_temp").innerHTML = obj.cell_1_temp.toFixed(2);;
    document.getElementById("cell_7_bal_state").innerHTML = obj.cell_1_bal_state;
    document.getElementById("cell_7_bal_temp").innerHTML = obj.cell_1_bal_temp.toFixed(2);;
    document.getElementById("cell_8_voltage").innerHTML = obj.cell_2_voltage.toFixed(2);;
    document.getElementById("cell_8_temp").innerHTML = obj.cell_2_temp.toFixed(2);;
    document.getElementById("cell_8_bal_state").innerHTML = obj.cell_2_bal_state;
    document.getElementById("cell_8_bal_temp").innerHTML = obj.cell_2_bal_temp.toFixed(2);;
    document.getElementById("cell_9_voltage").innerHTML = obj.cell_3_voltage.toFixed(2);;
    document.getElementById("cell_9_temp").innerHTML = obj.cell_3_temp.toFixed(2);;
    document.getElementById("cell_9_bal_state").innerHTML = obj.cell_3_bal_state;
    document.getElementById("cell_9_bal_temp").innerHTML = obj.cell_3_bal_temp.toFixed(2);;
    document.getElementById("time_awake_bmm_2").innerHTML = obj.time_awake;
    document.getElementById("time_waiting_bmm_2").innerHTML = obj.time_waiting;
    document.getElementById("mac_bmm_2").innerHTML = obj.mac_bmm;
    document.getElementById("last_msg_bmm_2").innerHTML = obj.last_msg;
  }, false);
}

function submitMessage() {
  alert("Saved values to ESP32 SPIFFS");
  setTimeout(function(){ document.location.reload(false); }, 500);   
}

function showPackInfo() {
  var x = document.getElementById("packInfo");
  if (x.style.display === "none") { x.style.display = "block"; } else { x.style.display = "none"; }
  document.getElementById("packConfig").style.display = "none";
}
function showPackConfig() {
  var x = document.getElementById("packConfig");
  if (x.style.display === "none") { x.style.display = "block"; } else { x.style.display = "none"; }
  var y = document.getElementById("packInfo"); y.style.display = "none";
}
function showModule1Info() {
  var x = document.getElementById("module1Info");
  if (x.style.display === "none") { x.style.display = "block"; } else { x.style.display = "none"; }
}
function showModule2Info() {
  var x = document.getElementById("module2Info");
  if (x.style.display === "none") { x.style.display = "block"; } else { x.style.display = "none"; }
}
function showModule3Info() {
  var x = document.getElementById("module3Info");
  if (x.style.display === "none") { x.style.display = "block"; } else { x.style.display = "none"; }
}

function eject_sd_card_config_btn_func() {
  if (document.getElementById("curr_lang").value == "en_us") {
    if (document.getElementById("eject_sd_card_config").value == "yes") {
      document.getElementById("eject_sd_card_config").value = "no";
      document.getElementById("eject_sd_card_config_btn").value = "No";
    } else if (document.getElementById("eject_sd_card_config").value == "no") {
      document.getElementById("eject_sd_card_config").value = "yes";
      document.getElementById("eject_sd_card_config_btn").value = "Yes";
    }
  } else if (document.getElementById("curr_lang").value == "pt_br") {
    if (document.getElementById("eject_sd_card_config").value == "yes") {
      document.getElementById("eject_sd_card_config").value = "no";
      document.getElementById("eject_sd_card_config_btn").value = "Não";
    } else if (document.getElementById("eject_sd_card_config").value == "no") {
      document.getElementById("eject_sd_card_config").value = "yes";
      document.getElementById("eject_sd_card_config_btn").value = "Sim";
    }
  }
}

function lang_config_btn_func() {
  if (document.getElementById("curr_lang").value == "en_us") {
    if (document.getElementById("lang_config").value == "en_us") {
      document.getElementById("lang_config").value = "pt_br";
      document.getElementById("lang_config_btn").value = "Portuguese";
    } else if (document.getElementById("lang_config").value == "pt_br") {
      document.getElementById("lang_config").value = "en_us";
      document.getElementById("lang_config_btn").value = "English";
    }
  } else if (document.getElementById("curr_lang").value == "pt_br") {
    if (document.getElementById("lang_config").value == "en_us") {
      document.getElementById("lang_config").value = "pt_br";
      document.getElementById("lang_config_btn").value = "Português";
    } else if (document.getElementById("lang_config").value == "pt_br") {
      document.getElementById("lang_config").value = "en_us";
      document.getElementById("lang_config_btn").value = "Inglês";
    }
  }
}

function update_lang_of_btns() {
  if (document.getElementById("curr_lang").value == "en_us") {
    if (document.getElementById("lang_config").value == "en_us") {
      document.getElementById("lang_config_btn").value = "English";
    } else if (document.getElementById("lang_config").value == "pt_br") {
      document.getElementById("lang_config_btn").value = "Portuguese";
    }
    if (document.getElementById("eject_sd_card_config").value == "yes") {
      document.getElementById("eject_sd_card_config_btn").value = "Yes";
    } else if (document.getElementById("eject_sd_card_config").value == "no") {
      document.getElementById("eject_sd_card_config_btn").value = "No";
    }
  } else if (document.getElementById("curr_lang").value == "pt_br") {
    if (document.getElementById("lang_config").value == "en_us") {
      document.getElementById("lang_config_btn").value = "Inglês";
      
    } else if (document.getElementById("lang_config").value == "pt_br") {
      document.getElementById("lang_config_btn").value = "Português";
    }
    if (document.getElementById("eject_sd_card_config").value == "yes") {
      document.getElementById("eject_sd_card_config_btn").value = "Sim";
    } else if (document.getElementById("eject_sd_card_config").value == "no") {
      document.getElementById("eject_sd_card_config_btn").value = "Não";
    }
  }
}

function changeLang(language) {
  var en_us = {
    nav_title: "myWBMS Dashboard",
    pack_voltage: "Pack Voltage",
    avg_cell_voltage: "Average Cell Voltage",
    pack_sob: "State of Battery",
    load_current: "Load Current",
    charging_current: "Charging Current",
    last_msg_bmu: "Last message: ",
    pack_voltage_BMU: "Pack voltage (BMU): ",
    charging_voltage: "Charging voltage: ",
    pack_voltage_BMM: "Pack voltage (BMMs): ",
    min_cell_voltage: "Min cell voltage: ",
    min_cell_voltage_release: "Min cell voltage release: ",
    max_cell_voltage: "Max cell voltage: ",
    max_cell_voltage_release: "Max cell voltage release: ",
    max_load_current: "Max load current: ",
    max_charging_current: "Max charging current: ",
    min_cell_temp: "Min cell temp: ",
    max_cell_temp: "Max cell temp: ",
    max_cell_bal_temp: "Max cell balancing temp: ",
    series_cell_num: "Number of cells in series: ",
    charging_switch_state: "Charging switch: ",
    discharging_switch_state: "Discharging switch: ",
    error_code_bmu: "Error code: ",
    min_cell_voltage_config: "Min cell voltage",
    min_cell_voltage_release_config: "Min cell voltage release",
    max_cell_voltage_config: "Max cell voltage",
    max_cell_voltage_release_config: "Max cell voltage release",
    max_load_current_config: "Max load current",
    max_charging_current_config: "Max charging current",
    min_cell_temp_config: "Min cell temp",
    max_cell_temp_config: "Max cell temp",
    max_cell_bal_temp_config: "Max cell bal temp",
    cell_1: "Cell #1",
    cell_2: "Cell #2",
    cell_3: "Cell #3",
    last_msg_bmm_0: "Last message: ",
    cell_1_info: "Cell #1 balancing: ",
    cell_2_info: "Cell #2 balancing: ",
    cell_3_info: "Cell #3 balancing: ",
    time_awake_bmm_0: "Time awake: ",
    time_waiting_bmm_0: "Time waiting: ",
    error_code_bmm_0: "Error code: ",
    cell_4: "Cell #4",
    cell_5: "Cell #5",
    cell_6: "Cell #6",
    last_msg_bmm_1: "Last message: ",
    cell_4_info: "Cell #4 balancing: ",
    cell_5_info: "Cell #5 balancing: ",
    cell_6_info: "Cell #6 balancing: ",
    time_awake_bmm_1: "Time awake: ",
    time_waiting_bmm_1: "Time waiting: ",
    error_code_bmm_1: "Error code: ",
    cell_7: "Cell #7",
    cell_8: "Cell #8",
    cell_9: "Cell #9",
    last_msg_bmm_2: "Last message: ",
    cell_7_info: "Cell #7 balancing: ",
    cell_8_info: "Cell #8 balancing: ",
    cell_9_info: "Cell #9 balancing: ",
    time_awake_bmm_2: "Time awake: ",
    time_waiting_bmm_2: "Time waiting: ",
    error_code_bmm_2: "Error code: ",
    highest_cell_voltage: "Highest cell voltage: ",
    lowest_cell_voltage: "Lowest cell voltage: ",
    submit: "Submit",
    eject_sd_card_config: "Eject SD card",
    lang_config: "Change language",
    pack_charge_capacity: "Charge capacity: ",
    pack_state_of_charge: "State of charge: ",
    pack_energy_capacity: "Energy capacity: ",
    pack_state_of_energy: "State of energy: ",
    charge_capacity_config: "Charge capacity",
    energy_capacity_config: "Energy capacity",
  }
  var pt_br = {
    nav_title: "Painel de Controle do myWBMS",
    pack_voltage: "Tensão da Bateria",
    avg_cell_voltage: "Tensão Média das Células",
    pack_sob: "Estado da Bateria",
    load_current: "Corrente da Carga",
    charging_current: "Corrente do Carregador",
    last_msg_bmu: "Última mensagem: ",
    pack_capacity: "Capacidade: ",
    pack_voltage_BMU: "Tensão da bateria (BMU): ",
    charging_voltage: "Tensão do carregador: ",
    pack_voltage_BMM: "Tensão da bateria (BMMs): ",
    min_cell_voltage: "Tensão mín. (células): ",
    min_cell_voltage_release: "Tensão mín. liberação (células): ",
    max_cell_voltage: "Tensão máx. (células): ",
    max_cell_voltage_release: "Tensão máx. liberação (células): ",
    max_load_current: "Corrente máx. da carga: ",
    max_charging_current: "Corrente máx. do carregador: ",
    min_cell_temp: "Temp. mín. (células): ",
    max_cell_temp: "Temp. máx. (células): ",
    max_cell_bal_temp: "Temp. máx. de balanc. (células): ",
    series_cell_num: "Número de células em série: ",
    charging_switch_state: "Estado da chave do carregador: ",
    discharging_switch_state: "Estado da chave da carga: ",
    error_code_bmu: "Código de erro: ",
    min_cell_voltage_config: "Tensão mín. (células)",
    min_cell_voltage_release_config: "Tensão mín. liberação (células)",
    max_cell_voltage_config: "Tensão máx. (células)",
    max_cell_voltage_release_config: "Tensão máx. liberação (células)",
    max_load_current_config: "Corrente máx. da carga",
    max_charging_current_config: "Corrente máx. do carregador",
    min_cell_temp_config: "Temp. mín. (células)",
    max_cell_temp_config: "Temp. máx. (células)",
    max_cell_bal_temp_config: "Temp. máx. de bal. (células)",
    cell_1: "Célula #1",
    cell_2: "Célula #2",
    cell_3: "Célula #3",
    last_msg_bmm_0: "Última mensagem: ",
    cell_1_info: "Célula #1 balanceamento: ",
    cell_2_info: "Célula #2 balanceamento: ",
    cell_3_info: "Célula #3 balanceamento: ",
    time_awake_bmm_0: "Tempo acordado: ",
    time_waiting_bmm_0: "Tempo esperando: ",
    error_code_bmm_0: "Código de erro: ",
    cell_4: "Célula #4",
    cell_5: "Célula #5",
    cell_6: "Célula #6",
    last_msg_bmm_1: "Última mensagem: ",
    cell_4_info: "Célula #4 balanceamento: ",
    cell_5_info: "Célula #5 balanceamento: ",
    cell_6_info: "Célula #6 balanceamento: ",
    time_awake_bmm_1: "Tempo acordado: ",
    time_waiting_bmm_1: "Tempo esperando: ",
    error_code_bmm_1: "Código de erro: ",
    cell_7: "Célula #7",
    cell_8: "Célula #8",
    cell_9: "Célula #9",
    last_msg_bmm_2: "Última mensagem: ",
    cell_7_info: "Célula #7 balanceamento: ",
    cell_8_info: "Célula #8 balanceamento: ",
    cell_9_info: "Célula #9 balanceamento: ",
    time_awake_bmm_2: "Tempo acordado: ",
    time_waiting_bmm_2: "Tempo esperando: ",
    error_code_bmm_2: "Código de erro: ",
    highest_cell_voltage: "Tensão mais alta (células): ",
    lowest_cell_voltage: "Tensão mais baixa (células): ",
    submit: "Salvar",
    eject_sd_card_config: "Ejetar cartão SD",
    lang_config: "Mudar idioma",
    pack_charge_capacity: "Capacidade de carga: ",
    pack_state_of_charge: "Estado de carga: ",
    pack_energy_capacity: "Capacidade de energia: ",
    pack_state_of_energy: "Estado de energia: ",
    charge_capacity_config: "Capacidade de carga",
    energy_capacity_config: "Capacidade de energia",
  }
  if (language == "en_us") {
    txt = en_us;
    document.getElementById("curr_lang").value = "en_us";
    document.getElementById("lang_config").value = "en_us";
    update_lang_of_btns();
  } else if (language == "pt_br") {
    txt = pt_br;
    document.getElementById("curr_lang").value = "pt_br";
    document.getElementById("lang_config").value = "pt_br";
    update_lang_of_btns();
  }
  document.getElementById("nav_title_txt").innerHTML = txt.nav_title;
  document.getElementById("pack_voltage_txt").innerHTML = txt.pack_voltage;
  document.getElementById("avg_cell_voltage_txt").innerHTML = txt.avg_cell_voltage;
  document.getElementById("pack_sob_txt").innerHTML = txt.pack_sob;
  document.getElementById("pack_charge_capacity_txt").innerHTML = txt.pack_charge_capacity;
  document.getElementById("pack_state_of_charge_txt").innerHTML = txt.pack_state_of_charge;
  document.getElementById("pack_energy_capacity_txt").innerHTML = txt.pack_energy_capacity;
  document.getElementById("pack_state_of_energy_txt").innerHTML = txt.pack_state_of_energy;
  document.getElementById("load_current_txt").innerHTML = txt.load_current;
  document.getElementById("charging_current_txt").innerHTML = txt.charging_current;
  document.getElementById("last_msg_bmu_txt").innerHTML = txt.last_msg_bmu;
  document.getElementById("pack_voltage_BMU_txt").innerHTML = txt.pack_voltage_BMU;
  document.getElementById("charging_voltage_txt").innerHTML = txt.charging_voltage;
  document.getElementById("pack_voltage_BMM_txt").innerHTML = txt.pack_voltage_BMM;
  document.getElementById("min_cell_voltage_txt").innerHTML = txt.min_cell_voltage;
  document.getElementById("min_cell_voltage_release_txt").innerHTML = txt.min_cell_voltage_release;
  document.getElementById("max_cell_voltage_txt").innerHTML = txt.max_cell_voltage;
  document.getElementById("max_cell_voltage_release_txt").innerHTML = txt.max_cell_voltage_release;
  document.getElementById("max_load_current_txt").innerHTML = txt.max_load_current;
  document.getElementById("max_charging_current_txt").innerHTML = txt.max_charging_current;
  document.getElementById("min_cell_temp_txt").innerHTML = txt.min_cell_temp;
  document.getElementById("max_cell_temp_txt").innerHTML = txt.max_cell_temp;
  document.getElementById("max_cell_bal_temp_txt").innerHTML = txt.max_cell_bal_temp;
  document.getElementById("series_cell_num_txt").innerHTML = txt.series_cell_num;
  document.getElementById("charging_switch_state_txt").innerHTML = txt.charging_switch_state;
  document.getElementById("discharging_switch_state_txt").innerHTML = txt.discharging_switch_state;
  document.getElementById("error_code_bmu_txt").innerHTML = txt.error_code_bmu;
  document.getElementById("min_cell_voltage_config_txt").innerHTML = txt.min_cell_voltage_config;
  document.getElementById("min_cell_voltage_release_config_txt").innerHTML = txt.min_cell_voltage_release_config;
  document.getElementById("max_cell_voltage_config_txt").innerHTML = txt.max_cell_voltage_config;
  document.getElementById("max_cell_voltage_release_config_txt").innerHTML = txt.max_cell_voltage_release_config;
  document.getElementById("max_load_current_config_txt").innerHTML = txt.max_load_current_config;
  document.getElementById("max_charging_current_config_txt").innerHTML = txt.max_charging_current_config;
  document.getElementById("charge_capacity_config_txt").innerHTML = txt.charge_capacity_config;
  document.getElementById("energy_capacity_config_txt").innerHTML = txt.energy_capacity_config;
  document.getElementById("min_cell_temp_config_txt").innerHTML = txt.min_cell_temp_config;
  document.getElementById("max_cell_temp_config_txt").innerHTML = txt.max_cell_temp_config;
  document.getElementById("max_cell_bal_temp_config_txt").innerHTML = txt.max_cell_bal_temp_config;
  document.getElementById("cell_1_txt").innerHTML = txt.cell_1;
  document.getElementById("cell_2_txt").innerHTML = txt.cell_2;
  document.getElementById("cell_3_txt").innerHTML = txt.cell_3;
  document.getElementById("last_msg_bmm_0_txt").innerHTML = txt.last_msg_bmm_0;
  document.getElementById("cell_1_info_txt").innerHTML = txt.cell_1_info;
  document.getElementById("cell_2_info_txt").innerHTML = txt.cell_2_info;
  document.getElementById("cell_3_info_txt").innerHTML = txt.cell_3_info;
  document.getElementById("time_awake_bmm_0_txt").innerHTML = txt.time_awake_bmm_0;
  document.getElementById("time_waiting_bmm_0_txt").innerHTML = txt.time_waiting_bmm_0;
  document.getElementById("error_code_bmm_0_txt").innerHTML = txt.error_code_bmm_0;
  document.getElementById("cell_4_txt").innerHTML = txt.cell_4;
  document.getElementById("cell_5_txt").innerHTML = txt.cell_5;
  document.getElementById("cell_6_txt").innerHTML = txt.cell_6;
  document.getElementById("last_msg_bmm_1_txt").innerHTML = txt.last_msg_bmm_1;
  document.getElementById("cell_4_info_txt").innerHTML = txt.cell_4_info;
  document.getElementById("cell_5_info_txt").innerHTML = txt.cell_5_info;
  document.getElementById("cell_6_info_txt").innerHTML = txt.cell_6_info;
  document.getElementById("time_awake_bmm_1_txt").innerHTML = txt.time_awake_bmm_1;
  document.getElementById("time_waiting_bmm_1_txt").innerHTML = txt.time_waiting_bmm_1;
  document.getElementById("error_code_bmm_1_txt").innerHTML = txt.error_code_bmm_1;
  document.getElementById("cell_7_txt").innerHTML = txt.cell_7;
  document.getElementById("cell_8_txt").innerHTML = txt.cell_8;
  document.getElementById("cell_9_txt").innerHTML = txt.cell_9;
  document.getElementById("last_msg_bmm_2_txt").innerHTML = txt.last_msg_bmm_2;
  document.getElementById("cell_7_info_txt").innerHTML = txt.cell_7_info;
  document.getElementById("cell_8_info_txt").innerHTML = txt.cell_8_info;
  document.getElementById("cell_9_info_txt").innerHTML = txt.cell_9_info;
  document.getElementById("time_awake_bmm_2_txt").innerHTML = txt.time_awake_bmm_2;
  document.getElementById("time_waiting_bmm_2_txt").innerHTML = txt.time_waiting_bmm_2;
  document.getElementById("error_code_bmm_2_txt").innerHTML = txt.error_code_bmm_2;
  document.getElementById("eject_sd_card_config_txt").innerHTML = txt.eject_sd_card_config;
  document.getElementById("lowest_cell_voltage_txt").innerHTML = txt.lowest_cell_voltage;
  document.getElementById("highest_cell_voltage_txt").innerHTML = txt.highest_cell_voltage;
  document.getElementById("submit_txt").value = txt.submit;
  document.getElementById("lang_config_txt").innerHTML = txt.lang_config;
}

</script>
</body>
</html>)rawliteral";
