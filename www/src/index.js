import React from 'react';
import ReactDOM from 'react-dom';
import { hashHistory } from 'react-router'
import { syncHistoryWithStore } from 'react-router-redux'
import { LocaleProvider } from 'antd'
import enUS from 'antd/lib/locale-provider/en_US'

import Root from './containers/Root'
import configureStore from './store/configureStore'
import './index.css';

const store = configureStore({
})
const history = syncHistoryWithStore(hashHistory, store)

ReactDOM.render(
    <LocaleProvider locale={enUS}>
        <Root store={store} history={history} />
    </LocaleProvider>,
    document.getElementById('root')
)
