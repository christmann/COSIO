/*
*
*   Author @Erik Funke
*
*/

function setConfigParameter(key, value){
  return new Promise(resolve => {
    var xhttp = new XMLHttpRequest();
    var url='setconfigparameter?' + key + "=" + value
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        if(xhttp.responseText == "OK"){
          resolve(true);
        }
        else if(xhttp.responseText == "Key Blacklisted"){
          alert("Key " + key + " is not modifyable");
          resolve(false);
        }
      }
    };
    xhttp.open("GET", url, true);
    xhttp.send();
  });
}

function getSignalStrength(){
  return new Promise(resolve => {
    var xhttp = new XMLHttpRequest();
    var url='getsignalstrength'
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        resolve(this.responseText);
      }
    };
    xhttp.open("GET", url, true);
    xhttp.send();
  });
}

function getConfigParameter(key, argument) {
  return new Promise(resolve => {
    var xhttp = new XMLHttpRequest();
    var url='getconfigparameter?' + key;
    if(argument){
      url += "=" + argument;
    }
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        resolve(this.responseText);
      }
    };
    xhttp.open("GET", url, true);
    xhttp.send();
  });
}

function setInHtml(key, updateField) {
  return new Promise(resolve => {
    var xhttp = new XMLHttpRequest();
    var url='getconfigparameter?' + key;

    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        document.getElementById(updateField).innerHTML = this.responseText;
        console.log("state change: updating " + updateField + " with " + this.responseText);
        resolve(this.responseText);
      }
    };

    xhttp.open("GET", url, true);
    xhttp.send();
  });
}

const keys = ["ssid", 
            "password", 
            "apSsid", 
            "apPassword", 
            "mdnsname", 
            "sensorId", 
            "sensorName", 
            "tempUnit", 
            "humidUnit", 
            "co2Unit", 
            "warningMessage", 
            "ppmMax",
            "warningInterval",
            "ppmNormal",
            "senseInterval",
            "apColor",
            "intensity",
            "showWarning",
            "active"];
