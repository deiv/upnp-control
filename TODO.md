
# TODO

- Advertise implementation:
  + Three discovery messages for the root device.
  + Two discovery messages for each embedded device.
  + Once for each service type in each device.

- Correct implementation of discovery:
    + **ssdp:all**: Search for all devices and services.
    + **upnp:rootdevice**: Search for root devices only.
    + **uuid:device-UUID**: Search for a particular device. Device UUID specified by UPnP vendor.
    + **urn:schemas-upnp-org:device:deviceType:v**: Search for any device of this type. Device type and version defined by UPnP Forum working committee.
    + **urn:schemas-upnp-org:service:serviceType:v**: Search for any service of this type. Service type and version defined by UPnP Forum working committee.
    + **urn:domain-name:device:deviceType:v**: Search for any device of this type. Domain name, device type and version defined by UPnP vendor. Period
characters in the domain name must be replaced with hyphens in accordance with RFC 2141.
    + **urn:domain-name:service:serviceType:v**: Search for any service of this type. Domain name, service type and version defined by UPnP vendor. Period
characters in the domain name must be replaced with hyphens in accordance with RFC 2141.
