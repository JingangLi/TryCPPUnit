source = """inline CppTL::StringConcatenator
operator +( %sleft, %sright ) {
   return CppTL::StringConcatenator( left, right );
}
"""
csz = 'const char *'
std_string = 'const std::string &'
types = [ csz, "const CppTL::ConstString &", "const CppTL::StringConcatenator &", "const CppTL::StringBuffer &", "const CppTL::ConstCharView &" ]
for left in xrange(0,len(types)):
    start = (left == 0) and 1 or 0
    for right_type in types[start:]:
        print  source % ( types[left], right_type )
    print

print '# ifndef CPPTL_CONSTSTRING_NO_STDSTRING'
types = types[1:] + [std_string]
for left_type in types:
    for right_type in types:
        if (left_type != std_string and right_type != std_string) or left_type == right_type:
            continue
        print  source % ( left_type, right_type )
print '# endif'
