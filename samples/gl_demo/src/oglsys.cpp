/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

#include <cstdint>
#include <memory>
#include <cstdio>
#include <cstdarg>

extern const char* s_applicationName;

#if defined(X11)
	#include <unistd.h>
	#include "X11/Xlib.h"
	#include "X11/Xutil.h"
#endif

#include "oglsys.hpp"

static struct OGLSysGlb {
#if defined(_WIN32)
	HINSTANCE mhInstance;
	ATOM mClassAtom;
	HWND mNativeWindow;
	//HDC mhDC;
#elif defined(UNIX)
	#if defined(X11)
		Display* mpNativeDisplay;
		Window mNativeWindow;
	#endif
#endif
	EGLNativeDisplayType mNativeDisplayHandle; // Win : HDC; X11 : Display

	uint64_t mFrameCnt;

	OGLSysIfc mIfc;
	int mWndOrgX;
	int mWndOrgY;
	int mWndW;
	int mWndH;
	int mWidth;
	int mHeight;

	GLint mDefFBO;
	GLint mMaxTexSize;

	void* mem_alloc(size_t size) {
		return mIfc.mem_alloc ? mIfc.mem_alloc(size) : malloc(size);
	}

	void mem_free(void* p) {
		if (mIfc.mem_free) {
			mIfc.mem_free(p);
		} else {
			free(p);
		}
	}

	void dbg_msg(const char* pFmt, ...) {
		if (mIfc.dbg_msg) {
			char buf[1024];
			va_list lst;
			va_start(lst, pFmt);
#ifdef _MSC_VER
			vsprintf_s(buf, sizeof(buf), pFmt, lst);
#else
			vsprintf(buf, pFmt, lst);
#endif
			mIfc.dbg_msg("%s\n", buf);
			va_end(lst);
		}
	}

	const char* load_glsl(const char* pPath, size_t* pSize) {
		if (mIfc.load_glsl) {
			return mIfc.load_glsl(pPath, pSize);
		} else {
			const char* pCode = nullptr;
			size_t size = 0;
			FILE* pFile = nullptr;
			const char* pMode = "rb";
#if defined(_MSC_VER)
			fopen_s(&pFile, pPath, pMode);
#else
			pFile = fopen(pPath, pMode);
#endif
			if (pFile) {
				if (fseek(pFile, 0, SEEK_END) == 0) {
					size = ftell(pFile);
				}
				fseek(pFile, 0, SEEK_SET);
				if (size) {
					pCode = (const char*)mem_alloc(size);
					if (pCode) {
						fread((void*)pCode, 1, size, pFile);
					}
				}
				fclose(pFile);
			}
			if (pSize) {
				*pSize = size;
			}
			return pCode;
		}
	}

	void unload_glsl(const char* pCode) {
		if (mIfc.unload_glsl) {
			mIfc.unload_glsl(pCode);
		} else {
			mem_free((void*)pCode);
		}
	}

	struct EGL {
		EGLDisplay display;
		EGLSurface surface;
		EGLContext context;
		EGLConfig config;

		void reset() {
			display = EGL_NO_DISPLAY;
			surface = EGL_NO_SURFACE;
			context = EGL_NO_CONTEXT;
		}
	} mEGL;

	bool valid_display() const { return mEGL.display != EGL_NO_DISPLAY; }
	bool valid_surface() const { return mEGL.surface != EGL_NO_SURFACE; }
	bool valid_context() const { return mEGL.context != EGL_NO_CONTEXT; }
	bool valid_ogl() const { return valid_display() && valid_surface() && valid_context(); }

	void stop_ogl() {
		if (valid_display()) {
			eglMakeCurrent(mEGL.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
			eglTerminate(mEGL.display);
		}
	}

	void init_sys();
	void init_wnd();
	void init_ogl();

	void reset_sys();
	void reset_wnd();
	void reset_ogl();

} G;

#if defined(_WIN32)

static LRESULT CALLBACK wnd_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	LRESULT res = 0;
	switch (msg) {
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_SYSCOMMAND:
			if (!(wParam == SC_SCREENSAVE || wParam == SC_MONITORPOWER)) {
				res = DefWindowProc(hWnd, msg, wParam, lParam);
			}
			break;
		default:
			res = DefWindowProc(hWnd, msg, wParam, lParam);
			break;
	}
	return res;
}

void OGLSysGlb::init_sys() {
	if (!GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCSTR)drwWndProc, &mhInstance)) {
		dbg_msg("Can't obtain instance handle");
	}
}

static const TCHAR* s_oglClassName = OGLSYS_ES ? _T("SysGLES") : _T("SysOGL");

void OGLSysGlb::init_wnd() {
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_VREDRAW | CS_HREDRAW;
	wc.hInstance = mhInstance;
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszClassName = s_oglClassName;
	wc.lpfnWndProc = wnd_proc;
	wc.cbWndExtra = 0x10;
	mClassAtom = RegisterClassEx(&wc);

	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = G.mWidth;
	rect.bottom = G.mHeight;
	int style = WS_CLIPCHILDREN | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_GROUP;
	AdjustWindowRect(&rect, style, FALSE);
	mWndW = rect.right - rect.left;
	mWndH = rect.bottom - rect.top;

	char tmpTitle[128];
	ZeroMemory(tmpTitle, sizeof(tmpTitle));
#if defined(_MSC_VER)
	_stprintf_s(tmpTitle, sizeof(tmpTitle)),
#else
	sprintf(tmpTitle,
#endif
		_T("%s: build %s"), s_applicationName, _T(__DATE__));
	size_t tlen = ::strlen(tmpTitle);
	TCHAR title[128];
	ZeroMemory(title, sizeof(title));
	for (size_t i = 0; i < tlen; ++i) {
		title[i] = (TCHAR)tmpTitle[i];
	}

	mNativeWindow = CreateWindowEx(0, s_oglClassName, title, style, mWndOrgX, mWndOrgY, mWndW, mWndH, NULL, NULL, mhInstance, NULL);
	if (mNativeWindow) {
		ShowWindow(mNativeWindow, SW_SHOW);
		UpdateWindow(mNativeWindow);
		mNativeDisplayHandle = GetDC(mNativeWindow);
	}
}

void ::reset_wnd() {
	if (mNativeDisplayHandle) {
		ReleaseDC(mNativeWindow, mNativeDisplayHandle);
		mhDC = NULL;
	}
	UnregisterClass(s_oglClassName, mhInstance);
}

#elif defined(X11)

static int wait_for_MapNotify(Display* pDisp, XEvent* pEvt, char* pArg) 
{
	if ((pEvt->type == MapNotify) && (pEvt->xmap.window == (Window)pArg)) { 
		return 1;
	}
	return 0;
}

void OGLSysGlb::init_sys() {}
void OGLSysGlb::reset_sys() {}

void OGLSysGlb::init_wnd() {
	using namespace std;

	dbg_msg("OGLSysGlb::init_wnd()");
	mpNativeDisplay = XOpenDisplay(0);
	if (mpNativeDisplay == 0) {
		dbg_msg("ERROR: can't open X display");
		return;
	}
	
	int defaultScreen = XDefaultScreen(mpNativeDisplay);
	int defaultDepth = DefaultDepth(mpNativeDisplay, defaultScreen);

	XVisualInfo visualInfo;
	Status status = XMatchVisualInfo(mpNativeDisplay, defaultScreen, defaultDepth, TrueColor, &visualInfo);

	if (status == 0) {
		dbg_msg("ERROR: can't aquire visual info");
		return;
	}

	Window rootWindow = RootWindow(mpNativeDisplay, defaultScreen);
	Colormap colorMap = XCreateColormap(mpNativeDisplay, rootWindow, visualInfo.visual, AllocNone);

	XSetWindowAttributes windowAttributes;
	windowAttributes.colormap = colorMap;
	windowAttributes.event_mask = StructureNotifyMask | ExposureMask | ButtonPressMask | KeyPressMask;
	mWndW = mWidth;
	mWndH = mHeight;
	mNativeWindow = XCreateWindow(mpNativeDisplay,
								rootWindow,
								0,
								0,
								mWndW,
								mWndH,
								0,
								visualInfo.depth,
								InputOutput,
								visualInfo.visual,
								CWEventMask | CWColormap,
								&windowAttributes);

	XMapWindow(mpNativeDisplay, mNativeWindow);
	XStoreName(mpNativeDisplay, mNativeWindow, s_applicationName);

	Atom windowManagerDelete = XInternAtom(mpNativeDisplay, "WM_DELETE_WINDOW", True);
	XSetWMProtocols(mpNativeDisplay, mNativeWindow, &windowManagerDelete , 1);

	mNativeDisplayHandle = (EGLNativeDisplayType)mpNativeDisplay;

	XEvent event;
	XIfEvent(mpNativeDisplay, &event, wait_for_MapNotify, (char*)mNativeWindow);
	dbg_msg("finished");
}

void OGLSysGlb::reset_wnd() {
	XDestroyWindow(mpNativeDisplay, mNativeWindow);
	XCloseDisplay(mpNativeDisplay);
}
#endif

void OGLSysGlb::init_ogl() {
	if (mNativeDisplayHandle) {
		mEGL.display = eglGetDisplay(mNativeDisplayHandle);
	}
	if (!valid_display()) return;
	int verMaj = 0;
	int verMin = 0;
	bool flg = eglInitialize(mEGL.display, &verMaj, &verMin);
	if (!flg) return;
	dbg_msg("EGL %d.%d\n", verMaj, verMin);
	flg = eglBindAPI(EGL_OPENGL_ES_API);
	if (flg != EGL_TRUE) return;

	static EGLint cfgAttrs[] = {
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
		EGL_DEPTH_SIZE, 24,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
		EGL_NONE
	};
	EGLint ncfg = 0;
	flg = eglChooseConfig(mEGL.display, cfgAttrs, &mEGL.config, 1, &ncfg);
	if (flg) flg = ncfg == 1;
	if (!flg) return;

	mEGL.surface = eglCreateWindowSurface(mEGL.display, mEGL.config,  EGLNativeWindowType(mNativeWindow), nullptr);
	if (!valid_surface()) return;

	static EGLint ctxAttrs[] = {
		EGL_CONTEXT_MAJOR_VERSION_KHR, 3,
		EGL_CONTEXT_MINOR_VERSION_KHR, 1,
		EGL_CONTEXT_CLIENT_VERSION, 3,
		EGL_NONE
	};
	mEGL.context = eglCreateContext(mEGL.display, mEGL.config, nullptr, ctxAttrs);
	if (!valid_context()) return;
	eglMakeCurrent(mEGL.display, mEGL.surface, mEGL.surface, mEGL.context);
	eglSwapInterval(mEGL.display, 1);

	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &mDefFBO);
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &mMaxTexSize);
}

void OGLSysGlb::reset_ogl() {
	mEGL.reset();
}

namespace OGLSys {

	bool s_initFlg = false;

	void init(const OGLSysCfg& cfg) {
		if (s_initFlg) return;
		memset(&G, 0, sizeof(G));
		//GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCSTR)wnd_proc, &G.mhInstance);
		G.mIfc = cfg.ifc;
		G.mWndOrgX = cfg.x;
		G.mWndOrgY = cfg.y;
		G.mWidth = cfg.width;
		G.mHeight = cfg.height;
		G.init_sys();
		G.init_wnd();
		G.init_ogl();
		s_initFlg = true;
	}

	void reset() {
		if (!s_initFlg) return;
		G.reset_ogl();
		G.reset_wnd();
		G.reset_sys();
		s_initFlg = false;
	}

	void stop() {
		G.stop_ogl();
	}

	void swap() {
		eglSwapBuffers(G.mEGL.display, G.mEGL.surface);
	}

#if defined(_WIN32)
	void loop(void(*pLoop)()) {
		MSG msg;
		bool done = false;
		while (!done) {
			if (PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE)) {
				if (GetMessage(&msg, NULL, 0, 0)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				} else {
					done = true;
					break;
				}
			} else {
				if (pLoop) {
					pLoop();
				}
				++G.mFrameCnt;
			}
		}
	}
#elif defined(X11)
	void loop(void(*pLoop)()) {
		XEvent event;
		bool done = false;
		while (!done) {
			KeySym key;
			while (XPending(G.mpNativeDisplay)) {
				XNextEvent(G.mpNativeDisplay, &event);
				switch (event.type) {
					case KeyPress:
						done = true;
				}
			}

			if (pLoop) {
				pLoop();
			}

		}
	}
#endif

	bool valid() {
		return G.valid_ogl();
	}

	void bind_def_framebuf() {
		if (valid()) {
			glBindFramebuffer(GL_FRAMEBUFFER, G.mDefFBO);
		}
	}

	int get_width() {
		return G.mWidth;
	}

	int get_height() {
		return G.mHeight;
	}

	uint64_t get_frame_count() {
		return G.mFrameCnt;
	}

	GLuint compile_shader_str(const char* pSrc, size_t srcSize, GLenum kind) {
		GLuint sid = 0;
		if (valid() && pSrc && srcSize > 0) {
			sid = glCreateShader(kind);
			if (sid) {
				GLint len[1] = { (GLint)srcSize };
				glShaderSource(sid, 1, (const GLchar* const*)&pSrc, len);
				glCompileShader(sid);
				GLint status = 0;
				glGetShaderiv(sid, GL_COMPILE_STATUS, &status);
				if (!status) {
					GLint infoLen = 0;
					glGetShaderiv(sid, GL_INFO_LOG_LENGTH, &infoLen);
					if (infoLen > 0) {
						char* pInfo = (char*)G.mem_alloc(infoLen);
						if (pInfo) {
							glGetShaderInfoLog(sid, infoLen, &infoLen, pInfo);
							G.dbg_msg("%s", pInfo);
							G.mem_free(pInfo);
							pInfo = nullptr;
						}
					}
					glDeleteShader(sid);
					sid = 0;
				}
			}
		}
		return sid;
	}

	GLuint compile_shader_file(const char* pSrcPath, GLenum kind) {
		GLuint sid = 0;
		if (valid()) {
			size_t size = 0;
			const char* pSrc = G.load_glsl(pSrcPath, &size);
			sid = compile_shader_str(pSrc, size, kind);
			G.unload_glsl(pSrc);
			pSrc = nullptr;
		}
		return sid;
	}

	GLuint link_prog(GLuint sidVert, GLuint sidFrag) {
		GLuint pid = 0;
		if (sidVert && sidFrag) {
			pid = glCreateProgram();
			if (pid) {
				glAttachShader(pid, sidVert);
				glAttachShader(pid, sidFrag);
				glLinkProgram(pid);
				GLint status = 0;
				glGetProgramiv(pid, GL_LINK_STATUS, &status);
				if (!status) {
					GLint infoLen = 0;
					glGetProgramiv(pid, GL_INFO_LOG_LENGTH, &infoLen);
					if (infoLen > 0) {
						char* pInfo = (char*)G.mem_alloc(infoLen);
						if (pInfo) {
							glGetProgramInfoLog(pid, infoLen, &infoLen, pInfo);
							G.dbg_msg("%s", pInfo);
							G.mem_free(pInfo);
							pInfo = nullptr;
						}
						glDeleteProgram(pid);
						pid = 0;
					}
				}
			}
		}
		return pid;
	}

} // OGLSys
