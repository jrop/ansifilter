#include <sstream>
#include <string>
// #include <node.h>
#include <nan.h>

std::string maskCharacter(unsigned char c) {
	if (c>0x1f || c=='\t') {
		return std::string( 1, c );
	} else {
		return "";
	}
}

std::string filter(std::string sin) {
	std::istringstream in(sin);
	std::ostringstream out;

	int cur=0;
	int next=0;

	std::string line;
	size_t i=0;
	bool isGrepOutput=false;

	while (true) {
		if (!getline(in, line)) break;

		i=0;
		size_t seqEnd=std::string::npos;

		while (i <line.length() ) {
			// CSI ?
			cur = line[i]&0xff;
			if (cur==0x1b || cur==0x9b || cur==0xc2) {
				if (line.length() - i > 2){

					next = line[i+1]&0xff;
					//move index behind CSI
					if ( (cur==0x1b && next==0x5b) || ( cur==0xc2 && next==0x9b) ) {
							++i;
					} else {
							// restore a unicode sequence if the two digit CSI is not matched
							// ansiweather -l Berlin,DE | ansifilter -T
							if (cur==0xc2 || cur==0x1b) out << maskCharacter(cur);
					}
					++i;
					if (line[i-1]==0x5b || (line[i-1]&0xff)==0x9b){
						seqEnd=i;
						//find sequence end
						while (   seqEnd<line.length()
							&& (line[seqEnd]<0x40 || line[seqEnd]>0x7e )) {
							++seqEnd;
							}

							isGrepOutput = line[seqEnd]=='K' && line[seqEnd-3] == 'm';
							// fix grep special K
							if (   line[seqEnd]=='s' || line[seqEnd]=='u'
								|| (line[seqEnd]=='K' && !isGrepOutput) )
								i=line.length();
							else
								i =   // ((line[seqEnd]=='m' || line[seqEnd]=='C'|| isGrepOutput) ?  1 : 0 )
								1 + ((seqEnd!=line.length())?seqEnd:i);
					} else {
						cur= line[i-1]&0xff;
						next = line[i]&0xff;

						//ignore content of two and single byte sequences (no CSI)
						if (cur==0x1b && (  next==0x50 || next==0x5d || next==0x58
							||next==0x5e||next==0x5f) )
						{
							seqEnd=i;
							//find string end
							while ( seqEnd<line.length() && (line[seqEnd]&0xff)!=0x9e
								&& line[seqEnd]!=0x07 ) {
								++seqEnd;
								}
								i=seqEnd+1;
						}
					}
				}
			} else if (cur==0x90 || cur==0x9d || cur==0x98 || cur==0x9e ||cur==0x9f) {
				seqEnd=i;
				//find string end
				while (   seqEnd<line.length() && (line[seqEnd]&0xff)!=0x9e
					&& line[seqEnd]!=0x07 ) {
					++seqEnd;
					}
					// handle false positives in unicode sequences
					// TODO fix set terminal title CSI (testansi.py)
					if (seqEnd<line.length() ) {
							i=seqEnd+1;
					} else {
						out << maskCharacter(line[i]);
						++i;
					}
			} else {
				// output printable character
				out << maskCharacter(line[i]);
				++i;
			}
		}
	} // while (true)
	out.flush();
	return out.str();
} // filter

// Expose to Node.JS {{
void Node_filter(const Nan::FunctionCallbackInfo<v8::Value>& args) {
	std::string sin = *(v8::String::Utf8Value)args[0]->ToString(Nan::GetCurrentContext()).ToLocalChecked();
	args.GetReturnValue().Set(Nan::New(filter(sin)).ToLocalChecked());
}
void Init(v8::Local<v8::Object> exports) {
	exports->Set(Nan::New("filter").ToLocalChecked(),
		Nan::New<v8::FunctionTemplate>(Node_filter)->GetFunction());
}
NODE_MODULE(ansifilter, Init)
// }}
