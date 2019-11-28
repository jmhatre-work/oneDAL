/* file: quality_metric_set_parameter.cpp */
/*******************************************************************************
* Copyright 2014-2019 Intel Corporation
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
#include "daal.h"
#include "com_intel_daal_algorithms_pca_quality_metric_set_QualityMetricSetParameter.h"

using namespace daal::algorithms::pca::quality_metric_set;

/*
* Class:     com_intel_daal_algorithms_pca_quality_metric_set_QualityMetricSetParameter
* Method:    cSetNComponents
* Signature: (JJ)V
*/
JNIEXPORT void JNICALL Java_com_intel_daal_algorithms_pca_quality_1metric_1set_QualityMetricSetParameter_cSetNComponents(JNIEnv *, jobject,
                                                                                                                         jlong parAddr,
                                                                                                                         jlong nComponents)
{
    (*(Parameter *)parAddr).nComponents = nComponents;
}

/*
* Class:     com_intel_daal_algorithms_pca_quality_metric_set_QualityMetricSetParameter
* Method:    cGetNComponents
* Signature: (J)J
*/
JNIEXPORT jlong JNICALL Java_com_intel_daal_algorithms_pca_quality_1metric_1set_QualityMetricSetParameter_cGetNComponents(JNIEnv *, jobject,
                                                                                                                          jlong parAddr)
{
    return (jlong)(*(Parameter *)parAddr).nComponents;
}

/*
* Class:     com_intel_daal_algorithms_pca_quality_metric_set_QualityMetricSetParameter
* Method:    cSetNBetaReducedModel
* Signature: (JJ)V
*/
JNIEXPORT void JNICALL Java_com_intel_daal_algorithms_pca_quality_1metric_1set_QualityMetricSetParameter_cSetNFeatures(JNIEnv *, jobject,
                                                                                                                       jlong parAddr, jlong nFeatures)
{
    (*(Parameter *)parAddr).nFeatures = nFeatures;
}

/*
* Class:     com_intel_daal_algorithms_pca_quality_metric_set_QualityMetricSetParameter
* Method:    cGetNFeatures
* Signature: (J)J
*/
JNIEXPORT jlong JNICALL Java_com_intel_daal_algorithms_pca_quality_1metric_1set_QualityMetricSetParameter_cGetNFeatures(JNIEnv *, jobject,
                                                                                                                        jlong parAddr)
{
    return (jlong)(*(Parameter *)parAddr).nFeatures;
}
