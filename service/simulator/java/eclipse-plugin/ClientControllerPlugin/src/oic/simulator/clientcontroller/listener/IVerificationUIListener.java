/*
 * Copyright 2015 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package oic.simulator.clientcontroller.listener;

import org.oic.simulator.client.SimulatorRemoteResource.RequestType;

import oic.simulator.clientcontroller.remoteresource.RemoteResource;

/**
 * Interface through which the automatic verification status events are sent to
 * the UI listeners.
 */
public interface IVerificationUIListener {
    public void onVerificationStarted(RemoteResource resource,
            RequestType reqType);

    public void onVerificationAborted(RemoteResource resource,
            RequestType reqType);

    public void onVerificationCompleted(RemoteResource resource,
            RequestType reqType);
}
