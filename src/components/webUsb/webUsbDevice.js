class WebUsbDevice {
    constructor(vid, intf, endPtIn, endPtOut) {
        this.vid = vid;
        this.intf = intf;
        this.endPtIn = endPtIn;
        this.endPtOut = endPtOut;
        this.connected = false;
    }

    Port = (device) => {
        this.device = device;
        return this;
    }

    requestPort = () => {
        const filters = [
            { "vendorId" : this.vid},
        ];
        return navigator.usb.requestDevice({'filters':filters})
        .then(device => {
            this.Port(device);
        })
        .catch(error => {
            console.log(error);
        })
    };

    getPorts = () => {
        return navigator.usb.getDevices()
        .then(devices => {
            return devices.map(device => {
                return this.Port(device);
            })
        })
        .catch(error => {
            console.log(error);
        })
    }

    connect = () => {
        let readLoop = () => {
            this.device.transferIn(this.endPtIn, 64)
            .then(result => {
                console.log(result.data);
                readLoop();
            })
            .catch(error => {
                console.log(error);
            })
            
        }

        return this.device.open()
            .then(() => {
                if(this.device.configuration === null) {
                    return this.device.selectConfiguration(1);
                }
            })
            .then(() => {
                let configurationInterfaces = this.device.configuration.interfaces;
                configurationInterfaces.forEach(element => {
                    element.alternates.forEach(elementalt => {
                      if (elementalt.interfaceClass === 0xff) {
                        this.intf = element.interfaceNumber;
                        elementalt.endpoints.forEach(elementendpoint => {
                          if (elementendpoint.direction === "out") {
                            this.endPtOut = elementendpoint.endpointNumber;
                          }
                          if (elementendpoint.direction === "in") {
                            this.endPtIn =elementendpoint.endpointNumber;
                          }
                        })
                      }
                    })
                  })        
            })
            .then(() => this.device.claimInterface(this.intf))
            .then(() => this.device.selectAlternateInterface(this.intf, 0))
            .then(() => {
                this.device.controlTransferOut({
                        'requestType': 'class',
                        'recipient': 'interface',
                        'request': 0x22,
                        'value': 0x01,
                        'index': this.intf});
                this.connected = true;
            })
            .then(() => {
              readLoop();
            })
            .catch(error => {
                this.connected = false;
                console.log(error);
            })  
    }

    disconnect = () => {
        return this.device.controlTransferOut({
            'requestType': 'class',
            'recipient': 'interface',
            'request': 0x22,
            'value': 0x00,
            'index': this.intf
        })
        .then(() => {
            this.connected = false;
            this.device.close();
        })
    }

    send = (data) => {
        return this.device.transferOut(this.endPtOut, data);
    }
}
 
export default WebUsbDevice;