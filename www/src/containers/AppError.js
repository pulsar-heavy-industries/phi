import React from 'react'
import { connect } from 'react-redux'
import { Modal } from 'antd'
import { appHideError } from '../actions'


const mapStateToProps = (state) => {
    return {
        error: state.app.error,
    }
}

const mapDispatchToProps = (dispatch) => {
    return {
        onDismiss: () => {
            dispatch(appHideError())
        }
    }
}

class AppError extends React.Component {
    componentDidMount() {
        Modal.error({
            title: 'Error',
            content: this.props.error.toString(),
            onOk: this.props.onDismiss,
        })
    }

    render() {
        return null
    }
}

export default connect(mapStateToProps, mapDispatchToProps)(AppError)

