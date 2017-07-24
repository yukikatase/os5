#define FDC_DMA_BUF_ADDR   0x80000
#define FDC_DMA_BUF_SIZE   512

#define KBD_STATUS    0x64
#define KBD_DATA      0x60
#define KEY_UP_BIT    0x80
#define SCREEN_WIDTH  320

int fdc_running = 0;

/* �e��ݒ肪�I���ƁAboot2d.asm ���� boot() ���Ă΂��
 */
void boot() {
  register_handlers();

  /* ������ pingpong.exe ��ǂݍ���Ŏ��s���� */

  while (1)
    halt();
}

int fdc_handler() {
  out8(0x20, 0x66);	/* �t���b�s�[�f�B�X�N���荞�� (IRQ6) ���ēx�L���ɂ��� */
  fdc_running = 0;
}

int null_kbd_handler() {
  out8(0x20, 0x61);	/* �L�[�{�[�h���荞�� (IRQ1) ���ēx�L���ɂ��� */
  in8(KBD_DATA);        /* �L�[�E�R�[�h�̓ǂݍ��� */
}

int null_timer_handler() {
  out8(0x20, 0x60);	/* �^�C�}�[���荞�� (IRQ0) ���ēx�L���ɂ��� */
}

int syscall_handler(int* regs) {
  int a = regs[0];
  int b = regs[1];

  return 0;
}

/* ���荞�ݏ����֐���o�^����
 */
int register_handlers() {
  int* ptr = (int*)0x7e00;
  *ptr = (int)null_kbd_handler;
  *(ptr + 1) = (int)null_timer_handler;
  *(ptr + 2) = (int)fdc_handler;
  *(ptr + 4) = (int)syscall_handler;

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
    { 0x00, 0x41, 0xff, 0x01 },	/* 1 */
    { 0x43, 0x85, 0x89, 0x71 },	/* 2 */
    { 0x42, 0x81, 0x91, 0x6e },	/* 3 */
    { 0x38, 0x48, 0xff, 0x08 },	/* 4 */
    { 0xfa, 0x91, 0x91, 0x8e },	/* 5 */
    { 0x3e, 0x51, 0x91, 0x0e },	/* 6 */
    { 0xc0, 0x83, 0x8c, 0xf0 },	/* 7 */
    { 0x6e, 0x91, 0x91, 0x6e },	/* 8 */
    { 0x70, 0x89, 0x8a, 0x7c }	/* 9 */
  };

  int i, j;
  char* vram = (char*)0xa0000;
  char* map = bitmaps[num];
  vram += (y * SCREEN_WIDTH + x);

  for (i = 0; i < 8; i++) {
    for (j = 0; j < 4; j++) {
      char bits = map[j];
      if (bits & (0x80 >> i))
        *(vram + j) = color;
      else
	*(vram + j) = 0;
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
