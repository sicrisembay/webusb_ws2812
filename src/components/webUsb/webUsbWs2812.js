import WebUsbDevice from './webUsbDevice';

class WebUsbWs2812 extends WebUsbDevice {
    ledCount = 0;
    frameSeq = 0;
    ledColor = [];

    constructor(ledCount) {
        const webUsbDev_vid = 0xCAFE;
        const webUsbDev_interface = 2;
        const webUsbDev_EndPtIn = 3;
        const webUsbDev_EndPtOut = 3;

        super(webUsbDev_vid, webUsbDev_interface, webUsbDev_EndPtIn, webUsbDev_EndPtOut);

        this.ledCount = ledCount;
        this.frameSeq = 0;
        
        for(let i = 0; i < this.ledCount; i++) {
            this.ledColor[i] = {};
            this.ledColor[i].id = i;
            this.ledColor[i].r = 0;
            this.ledColor[i].g = 0;
            this.ledColor[i].b = 0;
        }
        console.log(this)
    }

    updateColor = (id, r, g, b) => {
        if(id >= this.ledCount) {
            id = this.ledCount - 1;
        }
        if(id < 0) {
            id = 0;
        }
        if(id < this.ledCount) {
            this.ledColor[id].r = r;
            this.ledColor[id].g = g;
            this.ledColor[id].b = b;
        } else {
            throw Error("Invalid LED id: " + id);
        }
    }

    clear = () => {
        for(let led = 0; led < this.ledCount; led++) {
            this.ledColor[led].r = 0;
            this.ledColor[led].g = 0;
            this.ledColor[led].b = 0;
        }
    }

    flush = () => {
        let convertColorToPayload = (id, r, g, b) => {
            const CMD_UPDATE_COLOR = 16;
            let payloadLen = 6;
            let payload = new Uint8Array(payloadLen);
            payload[0] = CMD_UPDATE_COLOR;
            payload[1] = id % 256;
            payload[2] = (id >> 8) % 256;
            payload[3] = r % 256;
            payload[4] = g % 256;
            payload[5] = b % 256;
            
            return payload
        }

        let getPacket = (payload) => {
            const SZ_FRAME_OVERHEAD = 10;
            let packetLen = payload.length + SZ_FRAME_OVERHEAD;
            let newPacket = new Uint8Array(packetLen);
            /* Sof */
            newPacket[0] = 255;
            /* Lenght */
            newPacket[1] = packetLen % 256;
            newPacket[2] = (packetLen >> 8) % 256;
            newPacket[3] = (packetLen >> 16) % 256;
            newPacket[4] = (packetLen >> 24) % 256;
            /* Sequence */
            newPacket[5] = (this.frameSeq) % 256;
            newPacket[6] = (this.frameSeq >> 8) % 256;
            newPacket[7] = (this.frameSeq >> 16) % 256;
            newPacket[8] = (this.frameSeq >> 24) % 256;
            this.frameSeq = this.frameSeq + 1;
            /* payload */
            for(let i = 0; i < payload.length; i++) {
                newPacket[9+i] = (payload[i]) % 256;
            }
            /* Checksum */
            let sum = 0;
            for(let i = 0; i < (packetLen - 1); i++) {
                sum = sum + newPacket[i];
            }
            newPacket[packetLen - 1] = (0 - sum) % 256;

            return newPacket;
        }

        if(this.connected) {
            let combinedPacket = [];
            for (let i = 0; i < this.ledCount; i++) {
                const payload = convertColorToPayload(this.ledColor[i].id, 
                                    this.ledColor[i].r,
                                    this.ledColor[i].g,
                                    this.ledColor[i].b);
                const packet = getPacket(payload);
                let newPacket = new Uint8Array(combinedPacket.length + packet.length);
                newPacket.set(combinedPacket);
                newPacket.set(packet, combinedPacket.length);
                combinedPacket = newPacket;
            }

            this.send(combinedPacket);
        }
    }
}

export default WebUsbWs2812;
