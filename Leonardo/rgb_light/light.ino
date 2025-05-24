// Version 0.1
// Author: Remko Kleinjan
// Date: 240525

int redPin = 5;
int greenPin = 3;
int bluePin = 6;

int fadeSpeed = 0;
float steps = 1020;

boolean run = true;
float r_up, r_down, pr, nr = 0;
float g_up, g_down, pg, ng = 0;
float b_up, b_down, pb, nb = 0;

void setup() {
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  
  analogWrite(redPin, 0);
  analogWrite(greenPin, 0);
  analogWrite(bluePin, 0);
}

void loop() {
  while (run == true) {
    fade(250, 250, 250);
    fadeSpeed = 118;
    fade(255, 60, 15);
    fadeSpeed = 470;
    fade(64, 15, 4);
    run = false;
  }
}

void fade(int nr, int ng, int nb) {

  for (int i = 0; i < steps; i++) {

    if (pr < nr) {
      r_up += ((nr - pr) / steps);
      analogWrite(redPin, round(r_up));
      r_down = r_up;
    }
    if (pr > nr) {
      r_down -= ((pr - nr) / steps);
      analogWrite(redPin, round(r_down));
      r_up = r_down;
    }

    if (pg < ng) {
      g_up += ((ng - pg) / steps);
      analogWrite(greenPin, round(g_up));
      g_down = g_up;
    }
    if (pg > ng) {
      g_down -= ((pg - ng) / steps);
      analogWrite(greenPin, round(g_down));
      g_up = g_down;
    }

    if (pb < nb) {
      b_up += ((nb - pb) / steps);
      analogWrite(bluePin, round(b_up));
      b_down = b_up;
    }
    if (pb > nb) {
      b_down -= ((pb - nb) / steps);
      analogWrite(bluePin, round(b_down));
      b_up = b_down;
    }

    delay(fadeSpeed);
  }

  pr = nr;
  pg = ng;
  pb = nb;

}
