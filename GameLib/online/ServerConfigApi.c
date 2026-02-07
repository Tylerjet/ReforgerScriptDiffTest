class ServerConfigApi
{
	proto native void SearchOnlineConfigs(BackendCallback callback);
	proto native int GetOnlineConfigs(out notnull array<ServerConfigMeta> configs);
	proto native void UploadConfig(string configName, BackendCallback callback);
	proto native ServerConfigMeta Download(string sId, BackendCallback callback);
}

class ServerConfigMeta
{
	proto native void Download(BackendCallback callback);
	proto native void Delete(BackendCallback callback);
}