/*
 * A dummy setup function.
 */
void setup() {
	write("Setup started", 13);
}

static int i = 100;

/*
 * A dummy loop function.
 */
int loop() {
	i++;
	return i;
}
