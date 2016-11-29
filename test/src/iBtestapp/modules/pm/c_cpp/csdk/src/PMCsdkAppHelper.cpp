/******************************************************************
 *
 * Copyright 2016 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      LICENSE-2.0" target="_blank">http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *
 ******************************************************************/
#include "../include/PMCsdkAppHelper.h"

int g_OwnDevCount = 0;
int g_UnownDevCount = 0;
bool g_CBInvoked = CALLBACK_NOT_INVOKED;
#define TAG "provisioningclient"

FILE* fopenProvManager(const char* path, const char* mode)
{
    OC_UNUSED(path);
    return fopen(SVR_DB_FILE_NAME, mode);
}

int waitCallbackRet(void)
{
    IOTIVITYTEST_LOG(DEBUG, "waitCallbackRet IN");

    for (int i = 0; CALLBACK_TIMEOUT > i; ++i)
    {
        IOTIVITYTEST_LOG(DEBUG, "waitCallbackRet Loop = %d", i);

        if (true == g_CBInvoked)
        {
            return CALLBACK_INVOKED;
        }

        sleep(DELAY_SHORT);

        if (OC_STACK_OK != OCProcess())
        {
            IOTIVITYTEST_LOG(ERROR, "OCStack process error");
            return CALLBACK_NOT_INVOKED;
        }
    }

    IOTIVITYTEST_LOG(DEBUG, "waitCallbackRet OUT");
    return CALLBACK_NOT_INVOKED;
}

void ownershipTransferCB(void* ctx, int nOfRes, OCProvisionResult_t* arr, bool hasError)
{
    if (!hasError)
    {
        IOTIVITYTEST_LOG(DEBUG, "Ownership Transfer SUCCEEDED - ctx: %s", (char* ) ctx);
        g_CBInvoked = true;
    }
    else
    {
        IOTIVITYTEST_LOG(DEBUG, "Ownership Transfer FAILED - ctx: %s", (char* ) ctx);
        printResultList((const OCProvisionResult_t*) arr, nOfRes);
    }
}

void provisionPairwiseCB(void* ctx, int nOfRes, OCProvisionResult_t* arr, bool hasError)
{
    if (!hasError)
    {
        IOTIVITYTEST_LOG(DEBUG, "Provision Pairwise SUCCEEDED - ctx: %s\n", (char* ) ctx);
        g_CBInvoked = true;
    }
    else
    {
        IOTIVITYTEST_LOG(DEBUG, "Provision Pairwise Failed - ctx: %s", (char* ) ctx);
        printResultList((const OCProvisionResult_t*) arr, nOfRes);
    }
}

void provisionCredCB(void* ctx, int nOfRes, OCProvisionResult_t* arr, bool hasError)
{
    if (!hasError)
    {
        IOTIVITYTEST_LOG(DEBUG, "Provision Credential SUCCEEDED - ctx: %s", (char* ) ctx);
        g_CBInvoked = true;
    }
    else
    {
        IOTIVITYTEST_LOG(DEBUG, "Provision Credential FAILED - ctx: %s", (char* ) ctx);
        printResultList((const OCProvisionResult_t*) arr, nOfRes);
    }
}

void provisionAclCB(void* ctx, int nOfRes, OCProvisionResult_t* arr, bool hasError)
{
    if (!hasError)
    {
        IOTIVITYTEST_LOG(DEBUG, "Provision ACL SUCCEEDED - ctx: %s", (char* ) ctx);
        g_CBInvoked = true;
    }
    else
    {
        IOTIVITYTEST_LOG(DEBUG, "Provision ACL FAILED - ctx: %s", (char* ) ctx);
        printResultList((const OCProvisionResult_t*) arr, nOfRes);
    }
}

void unlinkDevicesCB(void* ctx, int nOfRes, OCProvisionResult_t* arr, bool hasError)
{
    if (!hasError)
    {
        IOTIVITYTEST_LOG(DEBUG, "Unlink Devices SUCCEEDED - ctx: %s", (char* ) ctx);
        g_CBInvoked = true;
    }
    else
    {
        IOTIVITYTEST_LOG(DEBUG, "Unlink Devices FAILED - ctx: %s", (char* ) ctx);
        printResultList((const OCProvisionResult_t*) arr, nOfRes);
    }
}

void removeDeviceCB(void* ctx, int nOfRes, OCProvisionResult_t* arr, bool hasError)
{
    if (!hasError)
    {
        IOTIVITYTEST_LOG(DEBUG, "Remove Devices SUCCEEDED - ctx: %s", (char* ) ctx);
        g_CBInvoked = true;
    }
    else
    {
        IOTIVITYTEST_LOG(DEBUG, "Remove Devices FAILED - ctx: %s", (char* ) ctx);
        printResultList((const OCProvisionResult_t*) arr, nOfRes);
    }
}

static void inputPinCB(char* pin, size_t len)
{
    if (!pin || OXM_RANDOM_PIN_MIN_SIZE >= len)
    {
        IOTIVITYTEST_LOG(ERROR, "inputPinCB invalid parameters");
        return;
    }

    printf("   > INPUT PIN: ");
    for (int ret = 0; 1 != ret;)
    {
        ret = scanf("%8s", pin);
        for (; 0x20 <= getchar();)
            ; // for removing overflow garbages
              // '0x20<=code' is character region
    }
}

// CAPI for Provisioning Manager
bool initProvisionClient()
{
    IOTIVITYTEST_LOG(DEBUG, "[PMHelper] initProvisionClient IN");

    // initialize persistent storage for SVR DB
    static OCPersistentStorage pstStr;

    pstStr.open = fopenProvManager;
    pstStr.read = fread;
    pstStr.write = fwrite;
    pstStr.close = fclose;
    pstStr.unlink = unlink;

    if (OC_STACK_OK != OCRegisterPersistentStorageHandler(&pstStr))
    {
        IOTIVITYTEST_LOG(ERROR, "OCRegisterPersistentStorageHandler error");
        return false;
    }

    // initialize OC stack and provisioning manager
    if (OC_STACK_OK != OCInit(NULL, 0, OC_CLIENT_SERVER))
    {
        IOTIVITYTEST_LOG(ERROR, "OCStack init error");
        return false;
    }

    if (access(PRVN_DB_FILE_NAME, F_OK) != -1)
    {
        printf("************************************************************\n");
        printf("************Provisioning DB file already exists.************\n");
        printf("************************************************************\n");
    }
    else
    {
        printf("*************************************************************\n");
        printf("************No provisioning DB file, creating new************\n");
        printf("*************************************************************\n");
    }

    if (OC_STACK_OK != OCInitPM(PRVN_DB_FILE_NAME))
    {
        IOTIVITYTEST_LOG(ERROR, "OC_PM init error");
        return false;
    }

    // register callback function(s) to each OxM
    OTMCallbackData_t otmcb;

    otmcb.loadSecretCB = LoadSecretJustWorksCallback;
    otmcb.createSecureSessionCB = CreateSecureSessionJustWorksCallback;
    otmcb.createSelectOxmPayloadCB = CreateJustWorksSelectOxmPayload;
    otmcb.createOwnerTransferPayloadCB = CreateJustWorksOwnerTransferPayload;

    if (OC_STACK_OK != OCSetOwnerTransferCallbackData(OIC_JUST_WORKS, &otmcb))
    {
        IOTIVITYTEST_LOG(ERROR, "OCSetOwnerTransferCallbackData error: OIC_JUST_WORKS");
        return false;
    }
    otmcb.loadSecretCB = InputPinCodeCallback;
    otmcb.createSecureSessionCB = CreateSecureSessionRandomPinCallback;
    otmcb.createSelectOxmPayloadCB = CreatePinBasedSelectOxmPayload;
    otmcb.createOwnerTransferPayloadCB = CreatePinBasedOwnerTransferPayload;

    if (OC_STACK_OK != OCSetOwnerTransferCallbackData(OIC_RANDOM_DEVICE_PIN, &otmcb))
    {
        IOTIVITYTEST_LOG(ERROR, "OCSetOwnerTransferCallbackData error: OIC_RANDOM_DEVICE_PIN");
        return false;
    }
    SetInputPinCB(inputPinCB);

    IOTIVITYTEST_LOG(DEBUG, "[PMHelper] initProvisionClient OUT");
    return true;
}

/**
 * Helper Method for OCGetDevInfoFromNetwork
 */
bool discoverAllDevices(int nTime, OCProvisionDev_t** own_list, OCProvisionDev_t** unown_list,
        OCStackResult expectedResult)
{
    IOTIVITYTEST_LOG(DEBUG, "[PMHelper] discoverAllDevices In");

    OCStackResult res = OCGetDevInfoFromNetwork(nTime, own_list, unown_list);
    IOTIVITYTEST_LOG(INFO, "[PMHelper] OCGetDevInfoFromNetwork API returns: %s\n",
            getOCStackResult(res));

    if (expectedResult != res)
    {
        IOTIVITYTEST_LOG(ERROR, "[PMHelper]  Expected Error : %s Actual Error : %s\n",
                getOCStackResult(expectedResult), getOCStackResult(res));
        return false;
    }

    if (OC_STACK_OK == res)
    {

        IOTIVITYTEST_LOG(INFO, "[PMHelper] Discovered Owned Devices List :");
        g_OwnDevCount = printDevList(*own_list);

        IOTIVITYTEST_LOG(INFO, "[PMHelper] Discovered Unowned Devices List :");
        g_UnownDevCount = printDevList(*unown_list);

    }

    IOTIVITYTEST_LOG(DEBUG, "[PMHelper] discoverAllDevices OUT");
    return true;
}

/**
 * Helper Method for OCDiscoverUnownedDevices
 */
bool discoverUnownedDevices(int nTime, OCProvisionDev_t** unown_list, OCStackResult expectedResult)
{
    IOTIVITYTEST_LOG(DEBUG, "[PMHelper] discoverUnownedDevices IN");

    OCStackResult res = OCDiscoverUnownedDevices(nTime, unown_list);
    IOTIVITYTEST_LOG(INFO, "[PMHelper] OCDiscoverUnownedDevices API returns: %s\n",
            getOCStackResult(res));

    if (expectedResult != res)
    {
        IOTIVITYTEST_LOG(ERROR, "[PMHelper]  Expected Error : %s Actual Error : %s\n",
                getOCStackResult(expectedResult), getOCStackResult(res));
        return false;
    }

    printf("\n[PMHelper] Discovered Unwned Devices List :\n");

    if (unown_list != NULL)
    {
        g_UnownDevCount = printDevList(*unown_list);
    }

    IOTIVITYTEST_LOG(DEBUG, "[PMHelper] discoverUnownedDevices OUT");
    return true;
}

/**
 * Helper Method for OCDiscoverOwnedDevices
 */
bool discoverOwnedDevices(int nTime, OCProvisionDev_t** own_list, OCStackResult expectedResult)
{
    IOTIVITYTEST_LOG(DEBUG, "[PMHelper] discoverOwnedDevices IN");

    OCStackResult res = OCDiscoverOwnedDevices(nTime, own_list);
    IOTIVITYTEST_LOG(INFO, "[PMHelper] OCDiscoverOwnedDevices API returns: %s\n",
            getOCStackResult(res));

    if (expectedResult != res)
    {
        IOTIVITYTEST_LOG(ERROR, "[PMHelper]  Expected Error : %s Actual Error : %s\n",
                getOCStackResult(expectedResult), getOCStackResult(res));
        return false;
    }

    if (own_list != NULL)
    {
        IOTIVITYTEST_LOG(INFO, "[PMHelper] [PMHelper] Discovered Owned Devices List :");
        g_OwnDevCount = printDevList(*own_list);
    }

    IOTIVITYTEST_LOG(DEBUG, "[PMHelper] discoverOwnedDevices OUT");
    return true;
}

/**
 * Helper Method for OCDoOwnershipTransfer
 */
bool doOwnerShipTransfer(void* ctx, OCProvisionDev_t** unown_list,
        OCProvisionResultCB resultCallback, OCStackResult expectedResult)
{
    IOTIVITYTEST_LOG(DEBUG, "[PMHelper] doOwnerShipTransfer IN");

    IOTIVITYTEST_LOG(INFO, "[PMHelper] Printing Unowned Device List to be owned IN");
    g_UnownDevCount = printDevList(*unown_list);

    g_CBInvoked = false;

    OCStackResult res = OCDoOwnershipTransfer(ctx, *unown_list, resultCallback);
    IOTIVITYTEST_LOG(INFO, "[PMHelper] OCDoOwnershipTransfer API returns: %s",
            getOCStackResult(res));

    if (expectedResult != res)
    {
        IOTIVITYTEST_LOG(ERROR, "[PMHelper]  Expected Error : %s Actual Error : %s\n",
                getOCStackResult(expectedResult), getOCStackResult(res));
        return false;
    }

    if (OC_STACK_OK == res)
    {
        if (CALLBACK_NOT_INVOKED == waitCallbackRet())
        {
            IOTIVITYTEST_LOG(ERROR, "[PMHelper]  Callback Not Invoked\n");
            return false;
        }
    }

    IOTIVITYTEST_LOG(DEBUG, "[PMHelper] doOwnerShipTransfer IN");
    return true;
}

/**
 * Helper Method for OCProvisionACL
 */
bool provisionACL(void* ctx, const OCProvisionDev_t* selectedDeviceInfo, OicSecAcl_t *acl,
        OCProvisionResultCB resultCallback, OCStackResult expectedResult)
{
    IOTIVITYTEST_LOG(DEBUG, "[PMHelper] OCProvisionACL IN");

    g_CBInvoked = false;

    OCStackResult res = OCProvisionACL(ctx, selectedDeviceInfo, acl, resultCallback);
    IOTIVITYTEST_LOG(INFO, "[PMHelper]  OCProvisionACL API returns: %s\n", getOCStackResult(res));

    if (expectedResult != res)
    {
        IOTIVITYTEST_LOG(ERROR, "[PMHelper]  Expected Error : %s Actual Error : %s\n",
                getOCStackResult(expectedResult), getOCStackResult(res));
        return false;
    }

    if (res == OC_STACK_OK)
    {
        if (CALLBACK_NOT_INVOKED == waitCallbackRet())
        {
            IOTIVITYTEST_LOG(ERROR, "[PMHelper]  Callback Not Invoked\n");
            return false;
        }
    }

    IOTIVITYTEST_LOG(DEBUG, "[PMHelper] OCProvisionACL Out");
    return true;
}

/**
 * Helper Method for OCProvisionCredentials
 */
bool provisionCredentials(void *ctx, OicSecCredType_t type, size_t keySize,
        const OCProvisionDev_t *pDev1, const OCProvisionDev_t *pDev2,
        OCProvisionResultCB resultCallback, OCStackResult expectedResult)
{
    IOTIVITYTEST_LOG(DEBUG, "[PMHelper] provisionCredentials IN");

    g_CBInvoked = false;

    OCStackResult res = OCProvisionCredentials(ctx, type, keySize, pDev1, pDev2, resultCallback);
    IOTIVITYTEST_LOG(INFO, "[PMHelper]  OCProvisionCredentials API returns: %s\n",
            getOCStackResult(res));

    if (expectedResult != res)
    {
        IOTIVITYTEST_LOG(ERROR, "[PMHelper]  Expected Error : %s Actual Error : %s\n",
                getOCStackResult(expectedResult), getOCStackResult(res));
        return false;
    }

    if (res == OC_STACK_OK)
    {
        if (CALLBACK_NOT_INVOKED == waitCallbackRet())
        {
            IOTIVITYTEST_LOG(ERROR, "[PMHelper]  Callback Not Invoked\n");
            return false;
        }
    }

    IOTIVITYTEST_LOG(DEBUG, "[PMHelper] provisionCredentials OUT");
    return true;
}

/**
 * Helper Method for OCProvisionPairwiseDevices
 */
bool provisionPairwiseDevices(void* ctx, OicSecCredType_t type, size_t keySize,
        const OCProvisionDev_t *pDev1, OicSecAcl_t *pDev1Acl, const OCProvisionDev_t *pDev2,
        OicSecAcl_t *pDev2Acl, OCProvisionResultCB resultCallback, OCStackResult expectedResult)
{
    IOTIVITYTEST_LOG(DEBUG, "[PMHelper] provisionPairwiseDevices IN");

    g_CBInvoked = false;

    OCStackResult res = OCProvisionPairwiseDevices(ctx, type, keySize, pDev1, pDev1Acl, pDev2,
            pDev2Acl, resultCallback);
    IOTIVITYTEST_LOG(INFO, "[PMHelper]  OCProvisionPairwiseDevices API returns: %s\n",
            getOCStackResult(res));

    if (expectedResult != res)
    {
        IOTIVITYTEST_LOG(ERROR, "[PMHelper]  Expected Error : %s Actual Error : %s\n",
                getOCStackResult(expectedResult), getOCStackResult(res));
        return false;
    }

    if (OC_STACK_OK == res)
    {
        if (CALLBACK_NOT_INVOKED == waitCallbackRet())
        {
            IOTIVITYTEST_LOG(ERROR, "[PMHelper]  Callback Not Invoked\n");
            return false;
        }
    }

    IOTIVITYTEST_LOG(DEBUG, "[PMHelper] provisionPairwiseDevices OUT");
    return true;
}

/**
 * Helper Method for OCGetLinkedStatus
 */
bool getLinkedStatus(const OicUuid_t* uuidOfDevice, OCUuidList_t** uuidList, size_t* numOfDevices,
        OCStackResult expectedResult)
{

    IOTIVITYTEST_LOG(DEBUG, "[PMHelper] getLinkedStatus IN");

    OCStackResult res;

    res = OCGetLinkedStatus(uuidOfDevice, uuidList, numOfDevices);

    IOTIVITYTEST_LOG(INFO, "[PMHelper]  OCGetLinkedStatus API returns: %s\n",
            getOCStackResult(res));

    if (expectedResult != res)
    {
        IOTIVITYTEST_LOG(ERROR, "[PMHelper]  Expected Error : %s Actual Error : %s\n",
                getOCStackResult(expectedResult), getOCStackResult(res));
        return false;
    }

    if (OC_STACK_OK == res)
    {
        IOTIVITYTEST_LOG(INFO, "[PMHelper]  Total Linked Devices =  %d\n", *numOfDevices);
        printUuidList(*uuidList);
    }

    IOTIVITYTEST_LOG(DEBUG, "[PMHelper] getLinkedStatus OUT");
    return true;
}

/**
 * Helper Method for OCUnlinkDevices
 */
bool unlinkDevices(void* ctx, const OCProvisionDev_t* pTargetDev1,
        const OCProvisionDev_t* pTargetDev2, OCProvisionResultCB resultCallback,
        OCStackResult expectedResult)
{
    IOTIVITYTEST_LOG(DEBUG, "[PMHelper] unlinkDevices IN");

    g_CBInvoked = false;

    OCStackResult res = OCUnlinkDevices(ctx, pTargetDev1, pTargetDev2, resultCallback);
    IOTIVITYTEST_LOG(INFO, "[PMHelper]  OCUnlinkDevices API returns: %s\n", getOCStackResult(res));

    if (expectedResult != res)
    {
        IOTIVITYTEST_LOG(ERROR, "[PMHelper]  Expected Error : %s Actual Error : %s\n",
                getOCStackResult(expectedResult), getOCStackResult(res));
        return false;
    }

    if (OC_STACK_OK == res)
    {
        if (CALLBACK_NOT_INVOKED == waitCallbackRet())
        {
            IOTIVITYTEST_LOG(ERROR, "[PMHelper]  Callback Not Invoked\n");
            return false;
        }
    }

    IOTIVITYTEST_LOG(DEBUG, "[PMHelper] unlinkDevices OUT");
    return true;
}

/**
 * Helper Method for OCRemoveDevice
 */
bool removeDevice(void* ctx, unsigned short waitTimeForOwnedDeviceDiscovery,
        const OCProvisionDev_t* pTargetDev, OCProvisionResultCB resultCallback,
        OCStackResult expectedResult)
{
    IOTIVITYTEST_LOG(DEBUG, "[PMHelper] OCRemoveDevice IN");

    g_CBInvoked = false;

    OCStackResult res = OCRemoveDevice(ctx, waitTimeForOwnedDeviceDiscovery, pTargetDev,
            resultCallback);
    IOTIVITYTEST_LOG(INFO, "[PMHelper]  OCRemoveDevice API returns: %s\n", getOCStackResult(res));

    if (expectedResult != res)
    {
        IOTIVITYTEST_LOG(ERROR, "[PMHelper]  Expected Error : %s Actual Error : %s\n",
                getOCStackResult(expectedResult), getOCStackResult(res));
        return false;
    }

    if (OC_STACK_OK == res)
    {
        if (CALLBACK_NOT_INVOKED == waitCallbackRet())
        {
            IOTIVITYTEST_LOG(ERROR, "[PMHelper]  Callback Not Invoked\n");
            return false;
        }
    }

    IOTIVITYTEST_LOG(DEBUG, "[PMHelper] OCRemoveDevice OUT");
    return true;
}

OTMCallbackData_t otmCbRegister(int otmType)
{
    OTMCallbackData_t otmcb;

    switch (otmType)
    {
        case OTM_RANDOMPIN:
            otmcb.loadSecretCB = InputPinCodeCallback;
            otmcb.createSecureSessionCB = CreateSecureSessionRandomPinCallback;
            otmcb.createSelectOxmPayloadCB = CreatePinBasedSelectOxmPayload;
            otmcb.createOwnerTransferPayloadCB = CreatePinBasedOwnerTransferPayload;

            break;
        default: // JUSTWORK
            otmcb.loadSecretCB = LoadSecretJustWorksCallback;
            otmcb.createSecureSessionCB = CreateSecureSessionJustWorksCallback;
            otmcb.createSelectOxmPayloadCB = CreateJustWorksSelectOxmPayload;
            otmcb.createOwnerTransferPayloadCB = CreateJustWorksOwnerTransferPayload;
            break;
    }

    return otmcb;
}

OicSecAcl_t* createAcl(const int dev_num, int permission, OCProvisionDev_t** m_own_list)
{
    IOTIVITYTEST_LOG(DEBUG, "[PMHelper] createAcl In");

    printDevList(*m_own_list);

    OicSecAcl_t* acl = (OicSecAcl_t*) OICCalloc(1, sizeof(OicSecAcl_t));
    OicSecAce_t* ace = (OicSecAce_t*) OICCalloc(1, sizeof(OicSecAce_t));
    LL_APPEND(acl->aces, ace);

    int num = (dev_num == DEVICE_INDEX_TWO) ? DEVICE_INDEX_ONE : DEVICE_INDEX_TWO;

    OCProvisionDev_t* dev = getDevInst((OCProvisionDev_t*) *m_own_list, num);

    printDevList(dev);

    if (!dev || !dev->doxm)
    {
        IOTIVITYTEST_LOG(DEBUG, "[PMHelper] createAcl: device instance empty");
        return NULL;
    }

    num = 1;

    for (int i = 0; num > i; ++i)
    {
        OicSecRsrc_t* rsrc = (OicSecRsrc_t*) OICCalloc(1, sizeof(OicSecRsrc_t));

        // Resource URI
        size_t len = strlen(LIGHT_RESOURCE_URI_01) + 1; // '1' for null termination
        rsrc->href = (char*) OICCalloc(len, sizeof(char));
        OICStrcpy(rsrc->href, len, LIGHT_RESOURCE_URI_01);

        // Resource Type
        rsrc->typeLen = 1;
        rsrc->types = (char**) OICCalloc(rsrc->typeLen, sizeof(char*));
        for (size_t i = 0; i < rsrc->typeLen; i++)
        {
            rsrc->types[i] = OICStrdup(LIGHT_RESOURCE_URI_01);
        }

        rsrc->interfaceLen = 1;
        rsrc->interfaces = (char**) OICCalloc(rsrc->typeLen, sizeof(char*));
        for (size_t i = 0; i < rsrc->interfaceLen; i++)
        {
            rsrc->interfaces[i] = OICStrdup(LIGHT_RESOURCE_URI_01);
        }

        LL_APPEND(ace->resources, rsrc);
    }

    ace->permission = permission;

    IOTIVITYTEST_LOG(DEBUG, "[PMHelper] createAcl Out");
    return acl;
}
