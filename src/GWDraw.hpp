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
			GWColorTuple3f hemiSky;
			GWColorTuple3f hemiGround;
			GWVectorF hemiUp;

			void set_hemi_up(const GWVectorF& up) {
				hemiUp.normalize(up);
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
} // GPU