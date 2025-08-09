std::vector<Instance> instances;
enum	Instance_Endpoint_Tag {
	INSTANCE_ENDPOINT_CGI,
	INSTANCE_ENDPOINT_UPLOAD,
	INSTANCE_ENDPOINT_SERVE,
};

union	Instance_Endpoint_Data {
	Instance_Endpoint_CGI		cgi; // 64 bytes
	Instance_Endpoint_Serve		serve; // 4 bytes
	Instance_Endpoint_Upload	upload; // 8 bytes
};

// CGI, Serve, Upload

struct	Instance_Endpoint { // Tagged union
	Instance_Endpoint_Tag	tag;
	Instance_Endpoint_Data	data;
}; // Runtime, 

Instance_Endpoint	x;
if (x.tag == INSTANCE_ENDPOINT_CGI) {
	x.data.cgi;
} else if (x.tag == INSTANCE_ENDPOINT_UPLOAD) {
	x.data.upload;
}

struct	Instance {
	std::string						addr;
	u64								port;
	std::vector<std::string>		hosts;
	Instance_Endpoint				endpoint; // 64 bytes
	std::vector<Instance_Endpoint>	endpoints;
};
