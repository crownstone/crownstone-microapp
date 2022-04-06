# Interaction bluenet and microapp

## First interaction
This is the first interaction between bluenet and a microapp.

```mermaid
%% https://mermaid-js.github.io/mermaid-live-editor/

sequenceDiagram
	participant fw as Bluenet
	participant fw_app as Microapp coroutine
	participant app as Microapp
	participant ipc as IPC (database)

	fw -->> ipc: setIpcRam(), store &microapp_callback

	fw ->> fw: startCoroutine()
	Note over fw: stores stack_params at g_RAM_MICROAPP_END
	Note over fw: stores reference to goIntoMicroapp

	fw ->> fw: callMicroapp()
	fw ->> fw_app: nextCoroutine()

    fw_app ->> app: goIntoMicroapp()
	Note over fw_app, app: cross-binary (jumps into microapp)
	Note over app: starts at ResetHandler (first instruction)

	app ->> app: main()
	app ->> app: setup()
	app ->> app: signalSetupEnd()
	app ->> app: sendMessage()

	app -->> ipc: getIpcRam(), get &microapp_callback

	app ->> fw_app: microapp_callback()
	Note over app,fw_app : cross-binary (jumps into bluenet)

	fw_app ->> fw_app: setCoroutineContext()
	Note over fw_app: store &io_buffer in stack_params

	fw_app ->> fw: yieldCoroutine()

	fw ->> fw: retrieveCommands()
```
## Tick calls
```mermaid
sequenceDiagram
	participant fw as Bluenet
	participant fw_app as Microapp coroutine
	participant app as Microapp
  Note over fw: Tick event
  fw ->> fw : tickMicroapp()
  loop while callAgain
	fw ->> fw : callMicroapp()
	fw ->> fw_app : nextCoroutine()
	Note over fw_app,app : Resume where last yielded
	fw_app ->> app : 
	Note over app : Continue microapp until sendMessage() call
	app ->> app : sendMessage()
	app ->> fw_app : microapp_callback()
	fw_app ->> fw_app : setCoroutineContext()
	fw_app ->> fw : yieldCoroutine()
	fw ->> fw : retrieveCommand()
	fw ->> fw : handleMicroappCommand()
  Note over fw: At end of setup, loop, or delay, stopAfterMicroappCommand returns true
  fw ->> fw : callAgain = !stopAfterMicroappCommand()
  end
  ```
## Interrupt calls
Todo
