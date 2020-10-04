import React from 'react';
import ReactDOM from 'react-dom';
import App from './components/App';
import { CSSReset, ThemeProvider } from '@chakra-ui/core';
import theme from './theme';

require('typeface-open-sans');
require('typeface-comfortaa');

ReactDOM.render(
  <React.StrictMode>
    <ThemeProvider theme={theme}>
      <CSSReset />
      <App />
    </ThemeProvider>
  </React.StrictMode>,
  document.getElementById('root')
);
