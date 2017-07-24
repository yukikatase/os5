#define KBD_STATUS    0x64
#define KBD_DATA      0x60
#define KEY_UP_BIT    0x80
#define SCREEN_WIDTH  320

int timer_counter = 0;
int blue[5];
int yellow[5];
int idle[5];
int blue_ends, yellow_ends;
int* running_thread;

/* �e��ݒ肪�I���ƁA�ŏ��� boot() ���Ă΂��
 */
void boot() {
  make_blue_thread();
  make_yellow_thread();
  running_thread = idle;
  register_handlers();

  while (1)
    halt();
}

int draw_yellow_line(int y, int interval) {
  int i;
  char* vram = (char*)0xa0000 + SCREEN_WIDTH * y;

  for (i = 0; i < 300; i++) {
    *vram++ = 14;
    int c = timer_counter;
    while (timer_counter < c + interval)
      halt();
  }

  yellow_ends = 1;
}

int draw_blue_line(int y, int interval) {
  int i;
  char* vram = (char*)0xa0000 + SCREEN_WIDTH * y;

  for (i = 0; i < 300; i++) {
    *vram++ = 3;
    int c = timer_counter;
    while (timer_counter < c + interval)
      halt();
  }

  blue_ends = 1;
}

int kbd_handler() {
  out8(0x20, 0x61);	/* �L�[�{�[�h���荞�� (IRQ1) ���ēx�L���ɂ��� */
  in8(KBD_DATA);
}

int make_blue_thread() {
  blue_ends = 0;
  make_thread(blue, 0x1ffff, draw_blue_line, 100, 5);
}

int make_yellow_thread() {
  yellow_ends = 0;
  make_thread(yellow, 0x2ffff, draw_yellow_line, 50, 1);
}

int timer_handler() {
  int* thread;
  int* old_thread;

  out8(0x20, 0x60);	/* �^�C�}�[���荞�� (IRQ0) ���ēx�L���ɂ��� */
  timer_counter++;

  if (running_thread == idle && blue_ends == 0)
    thread = blue;
  else
    thread = idle;

  old_thread = running_thread;
  running_thread = thread;
  sti();
  context_switch(old_thread, thread);
}

/* ���荞�ݏ����֐���o�^����
 */
int register_handlers() {
  int* ptr = (int*)0x7e00;
  *ptr = (int)kbd_handler;
  *(ptr + 1) = (int)timer_handler;

  out8(0x43,0x34);	/* timer */
  out8(0x40,0x9c);	/* 0x2e9c: 100Hz */
  out8(0x40,0x2e);

  sti();
  out8(0x21, 0xb8);	/* PIC0_IMR: accept only IRQ0,1,6 and IRQ2 (PIC1) */
  out8(0xa1, 0xff);	/* PIC1_IMR: no interrupt */
}

int print(int num, int x, int y, int color) {
  static char bitmaps[][4] = {
    { 0x7e, 0x81, 0x81, 0x7e },	/* 0 */
    { 0x00, 0x41, 0xff, 0x01 }	/* 1 */
  };

  int i, j;
  char* vram = (char*)0xa0000;
  char* map = bitmaps[num];
  vram += y * SCREEN_WIDTH;
  for (i = 0; i < 8; i++) {
    for (j = 0; j < 4; j++) {
      char bits = map[j];
      if (bits & (0x80 >> i))
        *(vram + y * 320 + x + j) = color;
    }

    vram += SCREEN_WIDTH;
  }

  return 0;
}

/* in ���߂� port �̒l (8bit) ��ǂ�
 */
int in8(int port) {
  int value;
  asm volatile ("mov $0, %%eax\n\tin %%dx,%%al"
                : "=r" (value) : "d" (port));
  return value;
}

/* out ���߂� port �ɒl (8bit) ����������
 */
int out8(int port, int value) {
  asm volatile ("out %%al,%%dx"
                : : "d" (port), "a" (value));
  return 0;
}

/* sti ���߂����s
 */
int sti() {
  asm volatile ("sti");
  return 0;
}

/* cli ���߂����s
 */
int cli() {
  asm volatile ("cli");
  return 0;
}

/* hlt ���߂Ńv���Z�b�T���~������
 */
int halt() {
  asm volatile ("hlt");
  return 0;
}

/* sti ���߂� hlt ���߂�A�����Ď��s
 * sti ���Ă��� hlt �܂ł̂킸���̎��Ԃ�
 * ���荞�݂��������Ȃ��悤�ɂ���B
 */
int sti_and_halt() {
  asm volatile ("sti\n\thlt");
  return 0;
}
