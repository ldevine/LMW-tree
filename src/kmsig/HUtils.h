#ifndef UTILS_H
#define UTILS_H


class HUtils {

protected:
	
	

public:

	// Clean up objects with pointers stored in an STL container
	template<class Seq> static void purge(Seq& c) {
		typename Seq::iterator i;
		for(i = c.begin(); i != c.end(); ++i) {
			delete *i;
			*i = 0;
		}
	}
	
};

#endif	

