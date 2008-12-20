// AVS APE (Plug-in Effect) header

// base class to derive from
class C_RBASE {
	public:
		C_RBASE() { }
		virtual ~C_RBASE() { };
		virtual int render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)=0; // returns 1 if fbout has dest, 0 if framebuffer has dest
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent){return 0;};
		virtual char *get_desc()=0;
		virtual void load_config(unsigned char *data, int len) { }
		virtual int  save_config(unsigned char *data) { return 0; }
};

// lovely helper functions for blending
static unsigned int __inline BLEND(unsigned int a, unsigned int b)
{
	register unsigned int r,t;
	r=(a&255)+((b)&255);
	t=min(r,255);
	r=((a>>8)&255)+((b>>8)&255);
	t|=min(r,255)<<8;
	r=((a>>16)&255)+((b>>16)&255);
	return t|min(r,255)<<16;
}

static unsigned int __inline BLEND_AVG(unsigned int a, unsigned int b)
{
	return ((a>>1)&~((1<<7)|(1<<15)|(1<<23)))+((b>>1)&~((1<<7)|(1<<15)|(1<<23)));
}
