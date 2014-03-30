void setup()
{
  Serial.begin(115200);
  readPtr = 0;
  writePtr = 0;
}

char read_buffer[256];
char write_buffer[256];

unsigned char readPtr;
unsigned char writePtr;

char ADDR;
char ID;

void decode()
{
  unsigned char decodePtr = read;
  unsigned char a,b,c,d;
  unsigned char x,y,z;
  unsigned char len = readPtr;
  unsigned char ptrIn = 0;
  unsigned char ptrOut = 0;
  while(len)
  {
    a = read_buffer[ptrIn++];
    b = read_buffer[ptrIn++];
    c = read_buffer[ptrIn++];
    d = read_buffer[ptrIn++];
    
    x = (a << 2) | (b >> 4);
    y = ((b & 0x0f) << 4) | (c >> 2);
    z = ((c & 0x03) << 6) | d;
    if(len--) read_buffer[ptrOut++] = x;
    else break;
    if(len--) read_buffer[ptrOut++] = y;
    else break;
    if(len--) read_buffer[ptrOut++] = z;
    else break;
  }
  readPtr = ptrOut;
}

void encode()
{
  char out[256];
  unsigned char a, b, c;
  unsigned char ptrIn = 0;
  unsigned char ptrOut = 0;
  unsigned char len = writePtr;
  while(len)
  {
    if(len)
    {
      a = write_buffer[ptrIn++];
      len--;
  }else a = 0;
  if(len)
  {
    b = write_buffer[ptrIn++];
    len--;
  }else b = 0;
  if(len)
  {
    c = write_buffer[ptrIn++];
    len--;
  }else c = 0;
  out[ptrOut++] = '=' + (a >> 2);
  out[ptrOut++] = '=' + (((a & 0x03) << 4) | ((b & 0xf0) >> 4);
  out[ptrOut++] = '=' + (((b & 0x0f) << 2) | ((c & 0xc0) >> 6));
  out[ptrOut++] = '=' + (c & 0x3f);
  }
  for(int i = 0; i < ptrOut; ++i)
  {
    write_buffer[i] = out[i];
  }
  writePtr = ptrOut;
}



void loop()
{
  if(Serial.available() > 3)
  {
    ADDR = Serial.read();
    ID = Serial.read();
    while(Serial.available())
    {
      read_buffer[readPtr++] = Serial.read();
    }
    decode();
  }
}
