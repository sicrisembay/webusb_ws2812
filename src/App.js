import React from 'react';
import {Navbar, Button} from 'react-bootstrap';
import WebUsbWs2812 from './components/webUsb/webUsbWs2812'
import { SketchPicker } from 'react-color'
import reactCSS from 'reactcss'
import logo from './logo.svg';
import './App.css';

class App extends React.Component {
  state = {
    displayColorPicker : false,
    color : {
      r: '241',
      g: '112',
      b: '19',
      a: '1',
    }
  }
  constructor() {
    super();
    this.webDevice = new WebUsbWs2812(24);
  }

  connectButtonClick = () => {
    if(this.webDevice.connected) {
      this.webDevice.disconnect()
      .then(() => {
          console.log(this);
      })
      .then(() => {
          if(this.webDevice.connected) {
              console.log("Connected.")
              this.setState({connected: true});
          } else {
              console.log("Disconnected.")
              this.setState({connected: false});
          }
      })
      .catch(error => {
          console.log(error);
      })
    } else {
      this.webDevice.requestPort()
      .then(() => {
          this.webDevice.connect()
          .then(() => {
              console.log(this);
          })
          .then(() => {
              if(this.webDevice.connected) {
                  console.log("Connected.")
                  this.setState({connected: true});
              } else {
                  console.log("Disconnected.")
                  this.setState({connected: false});
              }
          })
          .catch(error => {
              console.log(error);
          })
      })
      .catch(error => {
          console.log(error);
      })
    }
  }

  handleClick = () => {
    this.setState({displayColorPicker : !this.state.displayColorPicker});
  }

  handleClose = () => {
    this.setState({displayColorPicker : false});
  }

  handleChange = (color) => {
    console.log(color);
    this.setState({color : color.rgb});
    for(let led=0; led < this.webDevice.ledCount; led++) {
      this.webDevice.updateColor(led,
            parseInt(color.rgb.r, 10),
            parseInt(color.rgb.g, 10),
            parseInt(color.rgb.b, 10));
    }
    this.webDevice.flush();
  }

  render() {
    const styles = reactCSS({
      'default': {
        color: {
          width: '36px',
          height: '14px',
          borderRadius: '2px',
          background: `rgba(${ this.state.color.r }, ${ this.state.color.g }, ${ this.state.color.b }, ${ this.state.color.a })`,
        },
        swatch: {
          padding: '5px',
          background: '#fff',
          borderRadius: '1px',
          boxShadow: '0 0 0 1px rgba(0,0,0,.1)',
          display: 'inline-block',
          cursor: 'pointer',
        },
        popover: {
          position: 'absolute',
          zIndex: '2',
        },
        cover: {
          position: 'fixed',
          top: '0px',
          right: '0px',
          bottom: '0px',
          left: '0px',
        },
      },
    });

    return (
      <div className="App">
        <header className="App-header">
          <Navbar>
            <Button onClick={this.connectButtonClick}>
              {
                this.webDevice.connected ? 'Disconnect' : 'Connect'
              }
            </Button>
            <div style={ styles.swatch } onClick={ this.handleClick }>
              <div style={ styles.color } />
            </div>
              { 
                this.state.displayColorPicker ? 
                  <div style={ styles.popover }>
                    <div style={ styles.cover } onClick={ this.handleClose }/>
                    <SketchPicker color={ this.state.color } onChange={ this.handleChange } />
                  </div> 
                  : null 
              } 
          </Navbar>
          <img src={logo} className="App-logo" alt="logo" />
          <p>
            Edit <code>src/App.js</code> and save to reload.
          </p>
          <a
            className="App-link"
            href="https://reactjs.org"
            target="_blank"
            rel="noopener noreferrer"
          >
            Learn React
          </a>
        </header>
      </div>
    );
  }
}

export default App;
