//
//  Copyright (C) 2015 Google, Inc.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at:
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//

#include "service/ipc/binder/IBluetooth.h"

#include <base/logging.h>
#include <binder/IServiceManager.h>
#include <binder/Parcel.h>

using android::defaultServiceManager;
using android::IBinder;
using android::interface_cast;
using android::IServiceManager;
using android::Parcel;
using android::sp;
using android::String16;

namespace ipc {
namespace binder {

// static
const char IBluetooth::kBluetoothServiceName[] = "bluetoothservice";

// static
android::sp<IBluetooth> IBluetooth::getClientInterface() {
  sp<IServiceManager> sm = defaultServiceManager();
  if (!sm.get()) {
    LOG(ERROR) << "Failed to obtain a handle to the default Service Manager";
    return nullptr;
  }

  sp<IBinder> binder = sm->getService(String16(kBluetoothServiceName));
  if (!binder.get()) {
    LOG(ERROR) << "Failed to obtain a handle to the Bluetooth service";
    return nullptr;
  }

  sp<IBluetooth> bt_iface = interface_cast<IBluetooth>(binder);
  if (!bt_iface.get()) {
    LOG(ERROR) << "Obtained invalid IBinder handle";
    return nullptr;
  }

  return bt_iface;
}

// BnBluetooth (server) implementation
// ========================================================

android::status_t BnBluetooth::onTransact(
    uint32_t code,
    const Parcel& data,
    Parcel* reply,
    uint32_t flags) {
  VLOG(2) << "IBluetooth transaction: " << code;
  if (!data.checkInterface(this))
    return android::PERMISSION_DENIED;

  switch (code) {
    case IS_ENABLED_TRANSACTION: {
      bool is_enabled = IsEnabled();
      reply->writeInt32(is_enabled);
      return android::NO_ERROR;
    }
    case GET_STATE_TRANSACTION: {
      int state = GetState();
      reply->writeInt32(state);
      return android::NO_ERROR;
    }
    case ENABLE_TRANSACTION: {
      bool result = Enable();
      reply->writeInt32(result);
      return android::NO_ERROR;
    }
    case DISABLE_TRANSACTION: {
      bool result = Disable();
      reply->writeInt32(result);
      return android::NO_ERROR;
    }
    case GET_ADDRESS_TRANSACTION: {
      std::string address = GetAddress();
      reply->writeCString(address.c_str());
      return android::NO_ERROR;
    }
    case GET_UUIDS_TRANSACTION:
      // TODO(armansito): Figure out how to handle a Java "ParcelUuid" natively.
      // Should we just change the code to pass strings or byte arrays?
      return android::INVALID_OPERATION;

    case SET_NAME_TRANSACTION: {
      std::string name(data.readCString());
      bool result = SetName(name);
      reply->writeInt32(result);
      return android::NO_ERROR;
    }
    case GET_NAME_TRANSACTION: {
      std::string name = GetName();
      reply->writeCString(name.c_str());
      return android::NO_ERROR;
    }
    default:
      return BBinder::onTransact(code, data, reply, flags);
  }
}

// BpBluetooth (client) implementation
// ========================================================

BpBluetooth::BpBluetooth(const sp<IBinder>& impl)
    : BpInterface<IBluetooth>(impl) {
}

bool BpBluetooth::IsEnabled() {
  Parcel data, reply;

  data.writeInterfaceToken(IBluetooth::getInterfaceDescriptor());
  remote()->transact(IBluetooth::IS_ENABLED_TRANSACTION, data, &reply);

  return reply.readInt32();
}

int BpBluetooth::GetState() {
  Parcel data, reply;

  data.writeInterfaceToken(IBluetooth::getInterfaceDescriptor());
  remote()->transact(IBluetooth::GET_STATE_TRANSACTION, data, &reply);

  return reply.readInt32();
}

bool BpBluetooth::Enable() {
  Parcel data, reply;

  data.writeInterfaceToken(IBluetooth::getInterfaceDescriptor());
  remote()->transact(IBluetooth::ENABLE_TRANSACTION, data, &reply);

  return reply.readInt32();
}

bool BpBluetooth::EnableNoAutoConnect() {
  Parcel data, reply;

  data.writeInterfaceToken(IBluetooth::getInterfaceDescriptor());
  remote()->transact(IBluetooth::ENABLE_NO_AUTO_CONNECT_TRANSACTION,
                     data, &reply);

  return reply.readInt32();
}

bool BpBluetooth::Disable() {
  Parcel data, reply;

  data.writeInterfaceToken(IBluetooth::getInterfaceDescriptor());
  remote()->transact(IBluetooth::DISABLE_TRANSACTION, data, &reply);

  return reply.readInt32();
}

std::string BpBluetooth::GetAddress() {
  Parcel data, reply;

  data.writeInterfaceToken(IBluetooth::getInterfaceDescriptor());
  remote()->transact(IBluetooth::GET_ADDRESS_TRANSACTION, data, &reply);

  return reply.readCString();
}

std::vector<bluetooth::UUID> BpBluetooth::GetUUIDs() {
  // TODO(armansito): Figure out what to do here.
  return std::vector<bluetooth::UUID>();
}

bool BpBluetooth::SetName(const std::string& name) {
  Parcel data, reply;

  data.writeInterfaceToken(IBluetooth::getInterfaceDescriptor());
  data.writeCString(name.c_str());
  remote()->transact(IBluetooth::SET_NAME_TRANSACTION, data, &reply);

  return reply.readInt32();
}

std::string BpBluetooth::GetName() {
  Parcel data, reply;

  data.writeInterfaceToken(IBluetooth::getInterfaceDescriptor());
  remote()->transact(IBluetooth::GET_NAME_TRANSACTION, data, &reply);

  return reply.readCString();
}

IMPLEMENT_META_INTERFACE(Bluetooth, IBluetooth::kBluetoothServiceName);

}  // namespace binder
}  // namespace ipc
