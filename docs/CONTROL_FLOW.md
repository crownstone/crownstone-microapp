# Control flow

The control flow between bluenet and the microapp can in essence be described as follows (ignoring the initialization and `setup` for the moment):

Once every microapp tick, bluenet yields control towards the microapp, hoping that at some point it will yield back. The microapp will call its `loop` function. Often, the loop function will perform some task that requires bluenet, e.g. printing something to serial. Microapp will perform what can be thought of as a 'soft' yield: control is yielded back to bluenet to handle the request (e.g. sending the serial payload to uart), but the microapp expects to be handed back control once bluenet finishes handling the request. Upon completion of `loop` (or on a `delay` call within `loop`) the microapp will do a 'hard' yield: it yields without the expectations of being handed back control right away. Bluenet will then stop handing control to the microapp until the next microapp tick.

Interrupts function almost the exact same way as described above, with the exception that the initial trigger to enter the microapp comes from some predefined event within bluenet other than a tick.

In most cases, bluenet functions exactly as described above. However, there are some exceptions.
Firstly, if the microapp is doing too many consecutive soft yields, bluenet will not hand control back to the microapp after handling the request. Instead, the microapp is only called again next microapp tick or interrupt.
Secondly, bluenet throttles how many interrupts are passed through to the microapp per tick. Only after the next microapp tick will the microapp be able to receive interrupts again.
Lastly, though this is not a part of the main control flow, a watchdog in bluenet will keep track when a microapp gets stuck or takes up too much time. It will then reboot and disable the microapp. These mechanisms together should ensure that the microapp cannot compromise the internal working of bluenet and the crownstone in general.

## Minimal example
Let's consider the following `loop()` in the microapp:
```
void loop() {
    Serial.println("Loop");
}
```
This is what is happening under the hood:

```mermaid
sequenceDiagram
    participant b as Bluenet
    participant c as Coroutine
    participant m as Microapp Library
    participant um as User-facing Microapp
    participant m2b as MicroappToBluenetBuffer
    participant b2m as BluenetToMicroappBuffer

    Note over b : A bluenet tick is the initial trigger.
    b ->> b : tickMicroapp()
    b -->> b2m : Write to shared buffer
    Note over b2m : sdkType = CONTINUE <br> ack = NO_REQUEST
    b ->> b : callMicroapp()
    b ->> c : nextCoroutine()
    Note over c,m : Resume in the sendMessage() call <br> of signalLoopEnd() of previous loop().
    c ->> m : enter microapp
    b2m -->> m : Read from shared buffer
    m ->> m : handleBluenetRequest()
    Note over m : handleBluenetRequest() returns <br> early because of NO_REQUEST.
    m ->> um : loop()
    um ->> m : Serial.println("Loop")
    m -->> m2b : Write to shared buffer
    Note over m2b: sdkType = SERIAL <br> ack = REQUEST
    m ->> m : sendMessage()
    m ->> c : microapp_callback()
    c ->> b : yieldCoroutine()
    b ->> b : retrieveCommand()
    m2b -->> b : Read from shared buffer
    b ->> b : handleMicroappCommand()
    Note over b : In handleMicroappCommand() the <br> serial print request is handled.
    b -->> m2b : Write to shared buffer
    Note over m2b : ack = SUCCESS
    b ->> b : stopAfterMicroappCommand()
    Note over b : stopAfterMicroappCommand() will <br> return false for sdkType SERIAL. <br> Hence, call microapp again.
    b -->> b2m : Write to shared buffer
    Note over b2m : sdkType = CONTINUE <br> ack = NO_REQUEST
    b ->> b : callMicroapp()
    b ->> c : nextCoroutine()
    Note over c,m : Resume in the sendMessage() call <br> of Serial.println().
    c ->> m : enter microapp
    b2m -->> m : Read from shared buffer
    m ->> m : handleBluenetRequest()
    Note over m : handleBluenetRequest() returns <br> early because of NO_REQUEST.
    m2b -->> m : Read from shared buffer
    Note over m : Check ack from bluenet to see if <br> serial request was successfull
    m ->> um : Serial.println() returns
    um ->> m : loop() returns
    m ->> m : signalLoopEnd()
    m -->> m2b : Write to shared buffer
    Note over m2b : sdkType = YIELD <br> ack = NO_REQUEST
    m ->> m : sendMessage()
    m ->> c : microapp_callback()
    c ->> b : yieldCoroutine()
    b ->> b : retrieveCommand()
    m2b -->> b : Read from shared buffer
    b ->> b : handleMicroappCommand()
    Note over b : handleMicroappCommand() does <br> nothing for sdkType YIELD.
    b ->> b : stopAfterMicroappCommand()
    Note over b : stopAfterMicroappCommand() will <br> return true for sdkType YIELD. <br> Hence, do not call microapp again.
    Note over b : tickMicroapp() ends.
```

## Interrupt example
Now, consider that an interruptHandler has been registered for e.g. incoming mesh messages. A very simple microapp for this could look like this:

```
void receivedMesh(MeshMsg msg) {
    Serial.println("Received mesh");
}

void setup() {
    Mesh.setIncomingMeshMsgHandler(receivedMesh);
    Mesh.listen();
}

void loop() {
    // empty
}
```
The following sequence diagram shows what will happen when a mesh message of the microapp type is received in bluenet.

```mermaid
sequenceDiagram
    participant b as Bluenet
    participant c as Coroutine
    participant m as Microapp Library
    participant um as User-facing Microapp
    participant m2b as MicroappToBluenetBuffer
    participant b2m as BluenetToMicroappBuffer

    Note over b : An event of type EVT_RECV_MESH_MSG <br> is the initial trigger.
    b ->> b : handleEvent()
    b ->> b : onReceivedMeshMessage()
    Note over b : If the mesh message is not of <br> the microapp type, return early.
    b ->> b : softInterruptMesh()
    b -->> b2m : Write to shared buffer
    Note over b2m : sdkType = MESH <br> ack = REQUEST <br> interruptType = RECV_MSG
    b ->> b : softInterrupt()
    b ->> b : callMicroapp()
    b ->> c : nextCoroutine()
    Note over c,m : Resume in the sendMessage() call <br> of signalLoopEnd() of previous loop().
    c ->> m : enter microapp
    b2m -->> m : Read from shared buffer
    m ->> m : handleBluenetRequest()
    alt interrupt stack full
        Note over m : If the interrupt stack is full, <br> the microapp returns early
        m -->> b2m : Write to shared buffer
        Note over b2m : ack = ERR_BUSY
        m -->> m2b : Write to shared buffer
        Note over m2b : sdkType = YIELD <br> ack = NO_REQUEST
        m ->> m : sendMessage()
        m ->> c : microapp_callback()
        c ->> b : yieldCoroutine()
        b2m -->> b : Read from shared buffer
        Note over b : softInterrupt() will recognize <br> the microapp is busy, and <br> refrain calling it again until <br> the microapp successfully yields again.
        b ->> b : retrieveCommand()
        m2b -->> b : Read from shared buffer
        b ->> b : handleMicroappCommand()
        Note over b : handleMicroappCommand() does <br> nothing for sdkType YIELD.
        b ->> b : stopAfterMicroappCommand()
        Note over b : stopAfterMicroappCommand() will <br> return true for sdkType YIELD. <br> Hence, do not call microapp again.
        Note over b : softInterrupt() ends.
    else interrupt stack not full
        Note over m : If there is space in the interrupt stack, <br> acknowledge bluenets interrupt
        m -->> b2m : Write to shared buffer
        Note over b2m : ack = WAIT_FOR_SUCCESS
        Note over m : handleBluenetRequest() copies <br> shared buffers to top of <br> request- and interrupt stacks.
        m ->> m : handleSoftInterrupt()
        Note over m : handleSoftInterrupt() identifies <br> the interrupt handler based on <br> sdkType = MESH and <br> interruptType = RECV_MSG.
        m ->> m : softInterruptMesh()
        m ->> um : receivedMesh()
        um ->> m : Serial.println("Received Mesh")
        m -->> m2b : Write to shared buffer
        Note over m2b: sdkType = SERIAL <br> ack = REQUEST
        m ->> m : sendMessage()
        m ->> c : microapp_callback()
        c ->> b : yieldCoroutine()
        b2m -->> b : Read from shared buffer
        Note over b : On a WAIT_FOR_SUCCESS, <br> continue the soft interrupt.
        b ->> b : retrieveCommand()
        m2b -->> b : Read from shared buffer
        b ->> b : handleMicroappCommand()
        Note over b : In handleMicroappCommand() the <br> serial print request is handled.
        b -->> m2b : Write to shared buffer
        Note over m2b : ack = SUCCESS
        b ->> b : stopAfterMicroappCommand()
        Note over b : stopAfterMicroappCommand() will <br> return false for sdkType SERIAL. <br> Hence, call microapp again.
        b ->> b : callMicroapp()
        b ->> c : nextCoroutine()
        Note over c,m : Resume in the sendMessage() call <br> of Serial.println().
        c ->> m : enter microapp
        b2m -->> m : Read from shared buffer
        m ->> m : handleBluenetRequest()
        Note over m : handleBluenetRequest() returns <br> early because ack is not REQUEST.
        m2b -->> m : Read from shared buffer
        Note over m : Check ack from bluenet to see if <br> serial request was successfull
        m ->> um : Serial.println() returns
        um ->> m : receivedMesh() returns
        m ->> m : softInterruptMesh() returns
        Note over m : The user handler or internal handler <br> may return a return code, e.g. SUCCESS
        m ->> m : handleSoftInterrupt() returns
        Note over m : Continue in handleBluenetRequest()
        Note over m : Clear interrupt stack entry <br> and copy top request stack entry <br> back to shared buffer
        m -->> m2b : Write to shared buffer
        Note over m2b : sdkType = YIELD <br> ack = NO_REQUEST
        m -->> b2m : Write to shared buffer
        Note over b2m : ack = SUCCESS
        m ->> m : sendMessage()
        m ->> c : microapp_callback()
        c ->> b : yieldCoroutine()
        b2m -->> b : Read from shared buffer
        Note over b : Bluenet recognizes successfull <br> handling of interrupt and does not <br> call retrieveCommand(). <br> Force stop softInterrupt()
    end
```

## Bluenet prohibits microapp call
Lastly, let's see how bluenet intervenes when the microapp does not yield quickly enough. Bluenet keeps track of consecutive calls to the microapp in a counter. The counter is reset when the microapp makes a hard yield, i.e. a yield at the end of `setup`, `loop`, a `delay` call or the end of an interrupt handler. If the counter reaches the constant `MICROAPP_MAX_NUMBER_CONSECUTIVE_MESSAGES`, bluenet will not directly call `callMicroapp` again. The microapp will then be called again on the next microapp tick or interrupt.

The following microapp will trigger this behaviour, if `NUMBER_SERIAL_CALLS` exceeds `MICROAPP_MAX_NUMBER_CONSECUTIVE_MESSAGES`:

```
void loop() {
    for(int i = 0; i < NUMBER_SERIAL_CALLS; i++) {
        Serial.println(i);
    }
}
```
The microapp will resume at `Serial.println()` on the next microapp tick.
However, it is possible that before the next microapp tick, an interrupt occurs. Hence, bluenet makes a request following a non-yielding request from the microapp. What happens then? The following sequence diagram shows this scenario. For clarity, let's assume that the event handler for receiving a mesh message will NOT send a message over serial like in the previous example, but instead sends a mesh message. (If we used a serial call like in the previous example, it might get confused with the serial call in `loop`)

```
void receivedMesh(MeshMsg msg) {
    MeshMsg reply = msg;
    Mesh.sendMeshMsg(reply);
}
```

```mermaid
sequenceDiagram
    participant b as Bluenet
    participant c as Coroutine
    participant m as Microapp Library
    participant um as User-facing Microapp
    participant m2b as MicroappToBluenetBuffer
    participant b2m as BluenetToMicroappBuffer

    Note over b : The first part of the diagram is <br> exactly as in the first diagram. <br> This diagram start at <br> the stopAfterMicroapp() call <br> that triggers the call limit.
    Note over b2m : sdkType = CONTINUE <br> ack = NO_REQUEST
    Note over m2b : sdkType = SERIAL <br> ack = REQUEST

    b ->> b : stopAfterMicroappCommand()
    Note over b : stopAfterMicroappCommand() will <br> return true because of the call limit. <br> Hence, do not call microapp again.
    Note over b : Before the next tick, an <br> EVT_RECV_MESH_MSG occurs
    b ->> b : handleEvent()
    b ->> b : onReceivedMeshMessage()
    Note over b : If the mesh message is not of <br> the microapp type, return early.
    b ->> b : softInterruptMesh()
    b -->> b2m : Write to shared buffer
    Note over b2m : sdkType = MESH <br> ack = REQUEST <br> interruptType = RECV_MSG
    b ->> b : softInterrupt()
    b ->> b : callMicroapp()
    b ->> c : nextCoroutine()
    Note over c,m : Resume in the sendMessage() call <br> of Serial.println().
    c ->> m : enter microapp
    b2m -->> m : Read from shared buffer
    m ->> m : handleBluenetRequest()
    m -->> b2m : Write to shared buffer
    Note over b2m : ack = WAIT_FOR_SUCCESS
    Note over m : handleBluenetRequest() copies <br> bluenet message to <br> empty entry in internal queue.
    m ->> m : handleSoftInterrupt()
    Note over m : handleSoftInterrupt() identifies <br> the interrupt handler based on <br> sdkType = MESH and <br> interruptType = RECV_MSG.
    m ->> m : softInterruptMesh()
    m ->> um : receivedMesh()
    um ->> m : Mesh.sendMeshMsg(reply)
    m -->> m2b : Write to shared buffer
    Note over m2b: sdkType = MESH <br> ack = REQUEST
    m ->> m : sendMessage()
    m ->> c : microapp_callback()
    c ->> b : yieldCoroutine()
    b2m -->> b : Read from shared buffer
    Note over b : On a WAIT_FOR_SUCCESS, <br> continue the soft interrupt.
    b ->> b : retrieveCommand()
    m2b -->> b : Read from shared buffer
    b ->> b : handleMicroappCommand()
    Note over b : In handleMicroappCommand() the <br> send mesh request is dispatched <br> to the mesh module.
    b -->> m2b : Write to shared buffer
    Note over m2b : ack = SUCCESS
    b ->> b : stopAfterMicroappCommand()
    Note over b : stopAfterMicroappCommand() will <br> return false for sdkType MESH. <br> Hence, call microapp again.
    b ->> b : callMicroapp()
    b ->> c : nextCoroutine()
    Note over c,m : Resume in the sendMessage() call <br> of Mesh.sendMeshMsg().
    c ->> m : enter microapp
    b2m -->> m : Read from shared buffer
    m ->> m : handleBluenetRequest()
    Note over m : handleBluenetRequest() returns <br> early because ack is not REQUEST.
    m2b -->> m : Read from shared buffer
    Note over m : Check ack from bluenet to see if <br> mesh request was successfull
    m ->> um : Mesh.sendMeshMsg() returns
    um ->> m : receivedMesh() returns
    m ->> m : softInterruptMesh() returns
    Note over m : The user handler or internal handler <br> may return a return code, e.g. SUCCESS
    m ->> m : handleSoftInterrupt() returns
    Note over m : Continue in handleBluenetRequest()
    Note over m : Clear internal queue entry.
    m -->> b2m : Write to shared buffer
    Note over b2m : ack = SUCCESS
    m -->> m2b : Write to shared buffer
    Note over m2b : sdkType = YIELD <br> ack = NO_REQUEST
    m ->> m : sendMessage()
    m ->> c : microapp_callback()
    c ->> b : yieldCoroutine()
    b2m -->> b : Read from shared buffer
    Note over b : Bluenet recognizes successfull <br> handling of interrupt.
    b ->> b : retrieveCommand()
    m2b -->> b : Read from shared buffer
    b ->> b : handleMicroappCommand()
    Note over b : handleMicroappCommand() does <br> nothing for sdkType YIELD.
    b ->> b : stopAfterMicroappCommand()
    Note over b : stopAfterMicroappCommand() will <br> return true for sdkType YIELD. <br> Hence, do not call microapp again.
    Note over b : softInterrupt() ends.
    Note over b : After some time, <br> a tick event happens.
    b ->> b : tickMicroapp()
    b -->> b2m : Write to shared buffer
    Note over b2m : sdkType = NONE <br> ack = NO_REQUEST
    b ->> b : callMicroapp()
    b ->> c : nextCoroutine()
    Note over c,m : Resume in the sendMessage() call <br> of Serial.println() of unfinished previous loop().
    c ->> m : enter microapp
    b2m -->> m : Read from shared buffer
    m ->> m : handleBluenetRequest()
    Note over m : handleBluenetRequest() returns <br> early because of NO_REQUEST.
    m2b -->> m : Read from shared buffer
    Note over m : Check ack from bluenet to see if <br> serial request was successfull
    Note over b,b2m : HERE WE GET UNEXPECTED BEHAVIOUR: <br> THE READ ACK IN THE MICROAPP2BLUENET HAS BEEN OVERWRITTEN IN THE MEANTIME
    m ->> um : Serial.println() returns
    um ->> m : loop() returns
    m ->> m : signalLoopEnd()
    m -->> m2b : Write to shared buffer
    Note over m2b : sdkType = YIELD <br> ack = NO_REQUEST
    m ->> m : sendMessage()
    m ->> c : microapp_callback()
    c ->> b : yieldCoroutine()
    b ->> b : retrieveCommand()
    m2b -->> b : Read from shared buffer
    b ->> b : handleMicroappCommand()
    Note over b : handleMicroappCommand() does <br> nothing for sdkType YIELD.
    b ->> b : stopAfterMicroappCommand()
    Note over b : stopAfterMicroappCommand() will <br> return true for sdkType YIELD. <br> Hence, do not call microapp again.
    Note over b : tickMicroapp() ends.

```