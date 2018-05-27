Feature: Rendering Control

  Background:
    * url 'http://'

  Scenario: Rendering Control

    Given path '/MusicControl1'
    And request
"""
<?xml version="1.0"?>
<s:Envelope xmlns:s="http://schemas.xmlsoap.org/soap/envelope/" s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/">
    <s:Body>
        <u:GetAvailableInputs xmlns:u="urn:schemas-upnp-org:service:RenderingControl:1">
        </u:GetAvailableInputs>
    </s:Body>
</s:Envelope>
"""
    When method POST
    Then match response ==
"""
<?xml version="1.0"?>
<s:Envelope xmlns:s="http://schemas.xmlsoap.org/soap/envelope/" s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/">
    <s:Body>
        <u:GetAvailableInputsResponse xmlns:u="urn:schemas-upnp-org:service:RenderingControl:1">
            <RetAvailableInputs><!--#uptime-->398 </RetAvailableInputs>
        </u:GetAvailableInputsResponse>
    </s:Body>
</s:Envelope>
"""