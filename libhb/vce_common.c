/* vce_common.c
 *
 * Copyright (c) 2003-2018 HandBrake Team
 * This file is part of the HandBrake source code.
 * Homepage: <http://handbrake.fr/>.
 * It may be used under the terms of the GNU General Public License v2.
 * For full terms see the file COPYING file or visit http://www.gnu.org/licenses/gpl-2.0.html
 */

#ifdef USE_VCE
#include "AMF/core/Factory.h"
#include "AMF/components/VideoDecoderUVD.h"
#include "hb.h"

AMF_RESULT check_component_available(const wchar_t *componentID)
{
    amf_handle          library = NULL;
    AMFInit_Fn          init_fun;
    AMFFactory         *factory = NULL;
    AMFContext         *context = NULL;
    AMFComponent       *encoder = NULL;
    AMFCaps            *encoderCaps = NULL;
    AMF_RESULT          result = AMF_FAIL;

    hb_log("VCE: Checking for hardware availability.");
    
    library = hb_dlopen(AMF_DLL_NAMEA);
    if(!library)
    {
        result =  AMF_FAIL;
        hb_log("VCE: VCE Library not found.");
        goto clean;
    }

    init_fun = (AMFInit_Fn)(hb_dlsym(library, AMF_INIT_FUNCTION_NAME));
    if(!init_fun)
    {
        result = AMF_FAIL;
        hb_log("VCE: Load Library Failed");
        goto clean;
    }

    result = init_fun(AMF_FULL_VERSION, &factory);
    if(result != AMF_OK)
    {
        hb_log("VCE: Init Failed");
        goto clean;
    }

    result = factory->pVtbl->CreateContext(factory, &context);
    if(result != AMF_OK)
    {
        hb_log("VCE: Context Failed");
        goto clean;
    }

    result = context->pVtbl->InitDX11(context, NULL, AMF_DX11_1);
    if (result != AMF_OK) {
        result = context->pVtbl->InitDX9(context, NULL);
        if (result != AMF_OK) {
            hb_log("VCE: DX11 and DX9 Failed");
            goto clean;
        }
    }

    result = factory->pVtbl->CreateComponent(factory, context, componentID, &encoder);
    if(result != AMF_OK)
    {
        hb_log("VCE: Encoder not available");
        goto clean;
    }

    result = encoder->pVtbl->GetCaps(encoder, &encoderCaps);
    
    hb_log("VCE: Result = %d", result);

clean:
    if (encoderCaps)
    {
        encoderCaps->pVtbl->Clear(encoderCaps);
        encoderCaps->pVtbl->Release(encoderCaps);
        encoderCaps = NULL;
    }
    if (encoder)
    {
        encoder->pVtbl->Terminate(encoder);
        encoder->pVtbl->Release(encoder);
        encoder = NULL;
    }
    if (context)
    {
        context->pVtbl->Terminate(context);
        context->pVtbl->Release(context);
        context = NULL;
    }
    if(library)
    {
        hb_dlclose(library);
    }

    return result;
}

int hb_vce_h264_available()
{
    return (check_component_available(AMFVideoDecoderUVD_H264_AVC) == AMF_OK) ? 1 : 0;
}

int hb_vce_h265_available()
{
    return (check_component_available(AMFVideoDecoderHW_H265_HEVC) == AMF_OK) ? 1 : 0;
}

#else

int hb_vce_h264_available()
{
    return 0;
}

int hb_vce_h265_available()
{
    return 0;
}

#endif // USE_QSV
