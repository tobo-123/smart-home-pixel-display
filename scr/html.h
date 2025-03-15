//HTML code for main menu

const char INDEX[] PROGMEM = R"string_literal(
<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Smart Home Pixel Display</title>
<style>
html {
  font-family: Arial;
}
.button {
  background-color: #008CBA;
  border: none;
  border-radius: 4px;
  color: white;
  padding: 16px 40px;  
  font-size: 24px;
  margin: 2px;
  cursor: pointer;
}
.button2 {
  background-color: #f44336;
  border: none;
  border-radius: 2px; 
  font-size: 16px;
  cursor: pointer;
}
</style>
</head>

<body style="background-color:black;">
<h1><font color="white">Smart Home Pixel Display</h1>
<br>

<div style="width: 350px; float: left;">
<center>
<a href="/log"><button class="button">&nbsp;&nbsp;Log&nbsp;&nbsp;</button></a>
<br>
<br>
<a href="/config"><button class="button">Config</button></a>
<br><br><br><br>
<div style="width: 20px; float: left;">&nbsp;
</div>

<div style="width: 115px; float: left;">
<form action="/deletelog">
<input type="submit" value="Delete Log" class="button2">
</form>

</div>

<div style="width: 80px; float: left;">
<form action="/restart">
<input type="submit" value="Restart" class="button2">
</form>
</div>

<div style="width: 100px; float: left;">
<form action="/register">
<input type="submit" value="Register" class="button2">
</form>
</div>

<br><br><br>
<span id="message"> </span>
<br><br>

Version 1.0 by tobo-123
</div>
</body>

<script>
  setInterval(function ( ) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        document.getElementById("message").innerHTML = this.responseText;
      }
    };
    xhttp.open("GET", "/message", true);
    xhttp.send();
  }, 1000 ) ;
</script>

</html>
)string_literal";


//HTML code for log page - header

const char LOG_HEADER[] PROGMEM = R"string_literal(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Smart Home Pixel Display</title>
  <style> html { font-family: Arial; }
  .button {
  background-color: #008CBA;
  border: none;
  border-radius: 4px;
  color: white;
  padding: 8px 20px;  
  font-size: 20px;
  margin: 2px;
  cursor: pointer;
}
</style>
</head>
<body style="background-color:black;">
  <p>
  <h1><font color="white">Smart Home Pixel Display</h1>
  <a href="/"><button class="button">&nbsp;Back&nbsp;</button></a><br><br><br>
  Log of user defined state changes<br><br>
)string_literal";


//HTML code for config page - header

const char CONFIG_HEADER[] PROGMEM = R"string_literal(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Smart Home Pixel Display - Configuration</title>
  <style> html { font-family: Arial; }
  .button {
  background-color: #008CBA;
  border: none;
  border-radius: 4px;
  color: white;
  padding: 8px 20px;  
  font-size: 20px;
  margin: 2px;
  cursor: pointer;
}
</style>
</head>
<body style="background-color:black;">
<p>
<h1><font color="white">Smart Home Pixel Display</h1>
<a href="/"><button class="button">&nbsp;Back&nbsp;</button></a><br><br><br>
Defined pixel squares and their numbers:<br><br>
)string_literal";


//HTML code for config page - mid section

const char CONFIG_MID[] PROGMEM = R"string_literal(
<p>&nbsp</p>
<p>&nbsp</p>
<p>Configuration of user defined states:</p>
<div style="width: 210px; float: left;">Name</div>
<div style="width: 25px; float: left;">#</div>
<div style="width: 20px; float: left;">F</div>
<div style="width: 20px; float: left;">B</div>Function
<form action="/config" method="post" enctype="text/plain">
)string_literal";


//HTML code for config page - end section

const char CONFIG_END[] PROGMEM = R"string_literal(
<br>
<input type="submit" value="Submit" class="button"></form><br>
Name = Name of your user defined state in the Bosch smart home app<br>
# = Number of pixel square. If no square is to be activated, use number 0<br>
F = Flashing?<br>B = Buzzer?<br>
Function = Special function to call upon state change
</p>
</body>
</html>
)string_literal";