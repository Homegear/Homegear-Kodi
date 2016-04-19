#!/bin/tclsh

#Kanal-EasyMode!

#source [file join $env(DOCUMENT_ROOT) config/easymodes/em_common.tcl]
source [file join /www/config/easymodes/em_common.tcl]

#Namen der EasyModes tauchen nicht mehr auf. Der Durchgängkeit werden sie hier noch definiert.
set PROFILES_MAP(0) "Experte"
set PROFILES_MAP(1) "TheOneAndOnlyEasyMode"

proc getCheckBox {type param value prn} {
  set checked ""
  if { $value } then { set checked "checked=\"checked\"" }
  set s "<input id='separate_$type\_$prn' type='checkbox' $checked value='dummy' name=$param/>"
  return $s
}

proc getMinValue {param} {
  global psDescr
  upvar psDescr descr
  array_clear param_descr
  array set param_descr $descr($param)
  set min [format {%1.1f} $param_descr(MIN)]
  return "$min"
}

proc getMaxValue {param} {
  global psDescr
  upvar psDescr descr
  array_clear param_descr
  array set param_descr $descr($param)
  set max [format {%1.1f} $param_descr(MAX)]
  return "$max"
}

proc getTextField {type param value prn} {
  global psDescr
  set elemId 'separate_$type\_$prn'

  set s "<input id=$elemId type=\"text\" size=\"30\" value=\"$value\" name=\"$param\" />"
  return $s
}

proc getPasswordField {type param value prn} {
  global psDescr
  set elemId 'separate_$type\_$prn'

  set s "<input id=$elemId type=\"password\" size=\"30\" value=\"$value\" name=\"$param\" />"
  return $s
}

proc getHiddenField {type param value prn} {
  global psDescr
  set elemId 'separate_$type\_$prn'

  set s "<input id=$elemId type=\"text\" value=\"$value\" name=\"$param\" style=\"display: none;\" />"
  return $s
}

proc getRadioButton {type param value int_value variable_value prn} {
  global psDescr
  set elemId 'separate_$type\_$prn'

  set checked ""
  if { $variable_value == $int_value } then { set checked "checked=\"checked\"" }
  set s "<label><input onclick=\"document.getElementById($elemId).value = '$int_value';\" type=\"radio\" name=\"radio_$prn\" value=\"$value\" $checked style=\"margin-top: -2px; vertical-align: middle;\" />$value &nbsp;&nbsp;</label>"
  return $s
}

proc getUnit {param} {
  global psDescr
  upvar psDescr descr
  array_clear param_descr
  array set param_descr $descr($param)
  set unit $param_descr(UNIT)

  if {$unit == "minutes"} {
   set unit "\${lblMinutes}"
  }

  if {$unit == "K"} {
    set unit "&#176;C"
  }

  return "$unit"
}

proc getMinMaxValueDescr {param} {
  global psDescr
  upvar psDescr descr
  array_clear param_descr
  array set param_descr $descr($param)
  set min $param_descr(MIN)
  set max $param_descr(MAX)

  # Limit float to 2 decimal places
  if {[llength [split $min "."]] == 2} {
    set min [format {%1.2f} $min]
    set max [format {%1.2f} $max]
  }
  return "($min - $max)"
}

proc getHelpIcon {title text x y} {
  set ret "<img src=\"/ise/img/help.png\" style=\"cursor: pointer; width:18px; height:18px; position:relative; top:2px\" onclick=\"showParamHelp('$title', '$text', '$x', '$y')\">"
  return $ret
}

proc set_htmlParams {iface address pps pps_descr special_input_id peer_type} {

  global env iface_url psDescr

  upvar PROFILES_MAP  PROFILES_MAP
  upvar HTML_PARAMS   HTML_PARAMS
  upvar PROFILE_PNAME PROFILE_PNAME
  upvar $pps          ps
  upvar $pps_descr    ps_descr

  set DEVICE "DEVICE"
  set hlpBoxWidth 450
  set hlpBoxHeight 160

  puts "<script type=\"text/javascript\">"
    puts "showParamHelp = function(title, text, x , y) {"
      puts "var width = (! isNaN(x)) ? x : 450;"
      puts "var height = (! isNaN(y)) ? y : 260;"
      puts "MessageBox.show(title, text,\"\" ,width , height);"
    puts "}"
  puts "</script>"
  
  array set psDescr [xmlrpc $iface_url($iface) getParamsetDescription [list string $address] [list string MASTER]]

  set prn 1
  set param LED_TYPE
  append HTML_PARAMS(separate_1) "<table class=\"ProfileTbl\">"
  
  append HTML_PARAMS(separate_1) "<tr><td style=\"width: 100px\">LED-Typen</td>"
  append HTML_PARAMS(separate_1)  "<td>[getHiddenField $DEVICE $param $ps($param) $prn][getRadioButton $DEVICE $param "RGBW" 0 $ps($param) $prn][getRadioButton $DEVICE $param "Wei&szlig;" 1 $ps($param) $prn][getRadioButton $DEVICE $param "Beide" 2 $ps($param) $prn]</td>"
  append HTML_PARAMS(separate_1) "</tr>"
  
  append HTML_PARAMS(separate_1) "</table>"
}

constructor
