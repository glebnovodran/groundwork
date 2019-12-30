/*
 * Author: Gleb Novodran <novodran@gmail.com>
 */

namespace GWDraw {

	struct Context {

		enum AmbientMode {
			NONE,
			CONST,
			HEMI,
		};

		struct Hemi {
			GWColorTuple3f sky;
			GWColorTuple3f ground;
			GWVectorF up;
			float exp;
			float gain;

			void set_hemi_up(const GWVectorF& v) {
				up.normalize(v);
			}
			void reset() {
				GWTuple::set(sky, 1.1f, 1.1f, 1.125f);
				GWTuple::set(ground, 0.125f, 0.08f, 0.06f);
				GWTuple::set(up, 0.0f, 1.0f, 0.0f);
				exp = 1.0f;
				gain = 1.0f;
			}
		};

		struct Ambient {
			Hemi hemi;
			GWColorTuple3f color;
			AmbientMode ambientMode;
		};

		struct Light {
			Ambient ambient;
		};

		GWCamera camera;
		Light light;
	};

	struct Interface {
		void (*init)();
		void (*reset)();
		int (*get_screen_width)();
		int (*get_screen_height)();

		void (*begin)(const GWColorF& clearColor);
		void (*end)();
		void (*batch)(GWModel* pMdl, const int batchIdx, const Context* pCtx);
	};
}