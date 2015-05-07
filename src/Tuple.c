#include "Cello.h"

static const char* Tuple_Name(void) {
  return "Tuple";
}

static const char* Tuple_Brief(void) {
  return "Basic Collection";
}

static const char* Tuple_Description(void) {
  return
    "The `Tuple` type provides a basic way to create a simple collection of "
    "objects. Its main use is the fact that it can be constructed on the "
    "stack using the `tuple` macro. This makes it suitable for a number of "
    "purposes such as use in functions that take a variable number of "
    "arguments."
    "\n\n"
    "Tuples can also be constructed on the heap and stored in collections. "
    "This makes them also useful as a simple _untyped_ list of objects."
    "\n\n"
    "Internally Tuples are just a `NULL` terminated array of pointers. This "
    "makes positional access fast, but many other operations slow including "
    "iteration and counting the number of elements. Due to this it is only "
    "recommended Tuples are used for small collections. ";
}

static const char* Tuple_Definition(void) {
  return
    "struct Tuple {\n"
    "  var* items;\n"
    "};\n";
}

static struct Example* Tuple_Examples(void) {
  
  static struct Example examples[] = {
    {
      "Usage",
      "var x = tuple($I(100), $I(200), $S(\"Hello\"));\n"
      "show(x);\n"
      "var y = tuple(Int, $I(10), $I(20));\n"
      "var z = new_with(Array, y);\n"
      "show(z);\n"
      "\n"
      "foreach (item in x) {\n"
      "  println(\"%$\", item);\n"
      "}\n"
    }, {NULL, NULL}
  };

  return examples;
  
}

static struct Method* Tuple_Methods(void) {
  
  static struct Method methods[] = {
    {
      "tuple", 
      "#define tuple(...)",
      "Construct a `Tuple` object on the stack."
    }, {NULL, NULL, NULL}
  };
  
  return methods;
}

static void Tuple_New(var self, var args) {
  struct Tuple* t = self;
  size_t nargs = len(args);
  
  t->items = malloc(sizeof(var) * (nargs+1));
  
#if CELLO_MEMORY_CHECK == 1
  if (t->items is NULL) {
    throw(OutOfMemoryError, "Cannot create Tuple, out of memory!");
  }
#endif
  
  for (size_t i = 0; i < nargs; i++) {
    t->items[i] = get(args, $I(i));
  }
  
  t->items[nargs] = NULL;
}

static void Tuple_Del(var self) {
  struct Tuple* t = self;
  
#if CELLO_ALLOC_CHECK == 1
  if (header(self)->alloc is (var)AllocStack
  or  header(self)->alloc is (var)AllocStatic) {
    throw(ValueError, "Cannot destruct Tuple, not on heap!");
  }
#endif
  
  free(t->items);
}

static void Tuple_Assign(var self, var obj) {
  struct Tuple* t = self;
  size_t nargs = len(obj);
  
#if CELLO_ALLOC_CHECK == 1
  if (header(self)->alloc is (var)AllocStack
  or  header(self)->alloc is (var)AllocStatic) {
    throw(ValueError, "Cannot reallocate Tuple, not on heap!");
  }
#endif
  
  t->items = realloc(t->items, sizeof(var) * (nargs+1));
  
#if CELLO_MEMORY_CHECK == 1
  if (t->items is NULL) {
    throw(OutOfMemoryError, "Cannot allocate Tuple, out of memory!");
  }
#endif
  
  for (size_t i = 0; i < nargs; i++) {
    t->items[i] = get(obj, $I(i));
  }
  
  t->items[nargs] = NULL;
}

static size_t Tuple_Len(var self) {
  struct Tuple* t = self;
  size_t i = 0;
  while (t->items[i] isnt NULL) { i++; }
  return i;
}

static var Tuple_Iter_Init(var self) {
  struct Tuple* t = self;
  return t->items[0];
}

static var Tuple_Iter_Next(var self, var curr) {
  struct Tuple* t = self;
  size_t i = 0;
  while (t->items[i] isnt NULL) {
    if (t->items[i] is curr) { return t->items[i+1]; }
    i++;
  }
  return NULL;
}

static var Tuple_Iter_Last(var self) {
  struct Tuple* t = self;
  return t->items[Tuple_Len(t)-1];
}

static var Tuple_Iter_Prev(var self, var curr) {
  struct Tuple* t = self;
  if (curr is t->items[0]) { return NULL; }
  size_t i = 0;
  while (t->items[i] isnt NULL) {
    if (t->items[i] is curr) { return t->items[i-1]; }
    i++;
  }
  return NULL;
}

static var Tuple_Get(var self, var key) {
  struct Tuple* t = self;

  int64_t i = c_int(key);

#if CELLO_BOUND_CHECK == 1
  if (i < 0 or i >= (int64_t)Tuple_Len(t)) {
    return throw(IndexOutOfBoundsError, 
      "Index '%i' out of bounds for Tuple of size %i.", 
      key, $I(Tuple_Len(t)));
  }
#endif
  
  return t->items[i];
}

static void Tuple_Set(var self, var key, var val) {
  struct Tuple* t = self;

  int64_t i = c_int(key);

#if CELLO_BOUND_CHECK == 1
  if (i < 0 or i >= (int64_t)Tuple_Len(t)) {
    throw(IndexOutOfBoundsError, 
      "Index '%i' out of bounds for Tuple of size %i.", 
      key, $I(Tuple_Len(t)));
    return;
  }
#endif

  t->items[i] = val;
}

static bool Tuple_Mem(var self, var item) {
  foreach (obj in self) {
    if (eq(obj, item)) { return true; }
  }
  return false;  
}

static void Tuple_Pop_At(var self, var key);

static void Tuple_Rem(var self, var item) {
  struct Tuple* t = self;
  size_t i = 0;
  while (t->items[i] isnt NULL) {
    if (eq(item, t->items[i])) {
      Tuple_Pop_At(self, $I(i));
      return;
    }
    i++;
  }
}

static int Tuple_Show(var self, var output, int pos) {
  pos = print_to(output, pos, "<'Tuple' At 0x%p tuple(", self);
  for(size_t i = 0; i < len(self); i++) {
    pos = print_to(output, pos, "%$", get(self, $I(i)));
    if (i < len(self)-1) { pos = print_to(output, pos, ", "); }
  }
  return print_to(output, pos, ")>");
}

static void Tuple_Push(var self, var obj) {
  
  struct Tuple* t = self;
  size_t nitems = Tuple_Len(t);
  
#if CELLO_ALLOC_CHECK == 1
  if (header(self)->alloc is (var)AllocStack
  or  header(self)->alloc is (var)AllocStatic) {
    throw(ValueError, "Cannot reallocate Tuple, not on heap!");
  }
#endif
  
  t->items = realloc(t->items, sizeof(var) * (nitems+2));
  
#if CELLO_MEMORY_CHECK == 1
  if (t->items is NULL) {
    throw(OutOfMemoryError, "Cannot grow Tuple, out of memory!");
  }
#endif
  
  t->items[nitems+0] = obj;
  t->items[nitems+1] = NULL;
  
}

static void Tuple_Pop(var self) {
  
  struct Tuple* t = self;
  size_t nitems = Tuple_Len(t);
  
#if CELLO_BOUND_CHECK == 1
  if (nitems is 0) {
    throw(IndexOutOfBoundsError, "Cannot pop. Tuple is empty!");
    return;
  }
#endif
  
#if CELLO_ALLOC_CHECK == 1
  if (header(self)->alloc is (var)AllocStack
  or  header(self)->alloc is (var)AllocStatic) {
    throw(ValueError, "Cannot reallocate Tuple, not on heap!");
  }
#endif
  
  t->items = realloc(t->items, sizeof(var) * nitems);
  t->items[nitems-1] = NULL;
  
}

static void Tuple_Push_At(var self, var obj, var key) {
  
  struct Tuple* t = self;
  size_t nitems = Tuple_Len(t);

  int64_t i = c_int(key);

#if CELLO_BOUND_CHECK == 1
  if (i < 0 or i >= (int64_t)nitems) {
    throw(IndexOutOfBoundsError,
      "Index '%i' out of bounds for Tuple of size %i.", key, $I(nitems));
  }
#endif  
  
#if CELLO_ALLOC_CHECK == 1
  if (header(self)->alloc is (var)AllocStack
  or  header(self)->alloc is (var)AllocStatic) {
    throw(ValueError, "Cannot reallocate Tuple, not on heap!");
  }
#endif
  
  t->items = realloc(t->items, sizeof(var) * (nitems+2));
  
#if CELLO_MEMORY_CHECK == 1
  if (t->items is NULL) {
    throw(OutOfMemoryError, "Cannot grow Tuple, out of memory!");
  }
#endif

  memmove(&t->items[i+1], &t->items[i+0], sizeof(var) * (nitems - (size_t)i));
  t->items[i] = obj;
  
}

static void Tuple_Pop_At(var self, var key) {

  struct Tuple* t = self;
  size_t nitems = Tuple_Len(t);

  int64_t i = c_int(key);
  
#if CELLO_BOUND_CHECK == 1
  if (i < 0 or i >= (int64_t)nitems) {
    throw(IndexOutOfBoundsError,
      "Index '%i' out of bounds for Tuple of size %i.", key, $I(nitems));
  }
#endif
  
  memmove(&t->items[i+0], &t->items[i+1], sizeof(var) * (nitems - (size_t)i));
  
#if CELLO_ALLOC_CHECK == 1
  if (header(self)->alloc is (var)AllocStack
  or  header(self)->alloc is (var)AllocStatic) {
    throw(ValueError, "Cannot reallocate Tuple, not on heap!");
  }
#endif
  
  t->items = realloc(t->items, sizeof(var) * nitems);
  
}

static void Tuple_Concat(var self, var obj) {

  struct Tuple* t = self;
  size_t nitems = Tuple_Len(t);
  size_t objlen = len(obj);
  
#if CELLO_ALLOC_CHECK == 1
  if (header(self)->alloc is (var)AllocStack
  or  header(self)->alloc is (var)AllocStatic) {
    throw(ValueError, "Cannot reallocate Tuple, not on heap!");
  }
#endif
  
  t->items = realloc(t->items, sizeof(var) * (nitems+1+objlen));
  
#if CELLO_MEMORY_CHECK == 1
  if (t->items is NULL) {
    throw(OutOfMemoryError, "Cannot grow Tuple, out of memory!");
  }
#endif
  
  size_t i = nitems;
  foreach (item in obj) {
    t->items[i] = item;
    i++;
  }
  
  t->items[nitems+objlen] = NULL;
  
}

static void Tuple_Clear(var self) {
  struct Tuple* t = self;
  
#if CELLO_ALLOC_CHECK == 1
  if (header(self)->alloc is (var)AllocStack
  or  header(self)->alloc is (var)AllocStatic) {
    throw(ValueError, "Cannot reallocate Tuple, not on heap!");
  }
#endif
  
  t->items = realloc(t->items, sizeof(var));
  t->items[0] = NULL;
}

static void Tuple_Mark(var self, var gc, void(*f)(var,void*)) {
  struct Tuple* t = self;
  size_t i = 0;
  while (t->items[i] isnt NULL) {
    f(gc, t->items[i]); i++;
  }
}

static void Tuple_Swap(struct Tuple* t, size_t i, size_t j) {
  var tmp = t->items[i];
  t->items[i] = t->items[j];
  t->items[j] = tmp;
}

static void Tuple_Reverse(var self) {
  struct Tuple* t = self;
  size_t nitems = Tuple_Len(self);
  for (size_t i = 0; i < nitems / 2; i++) {
    Tuple_Swap(t, i, nitems-1-i);
  }
}

static size_t Tuple_Sort_Partition(
  struct Tuple* t, int64_t l, int64_t r, bool(*f)(var,var)) {
  
  int64_t p = l + (r - l) / 2;
  var tmp = t->items[p];
  Tuple_Swap(t, p, r);
  
  int64_t s = l;
  for (int64_t i = l; i < r; i++) {
    if (f(t->items[i], tmp)) {
      Tuple_Swap(t, i, s);
      s++;
    }
  }
  
  Tuple_Swap(t, s, r);
  return s;
}

static void Tuple_Sort_Part(
  struct Tuple* t, int64_t l, int64_t r, bool(*f)(var,var)) {
  if (l < r) {
    int64_t s = Tuple_Sort_Partition(t, l, r, f);
    Tuple_Sort_Part(t, l, s-1, f);
    Tuple_Sort_Part(t, s+1, r, f);
  }
}

static void Tuple_Sort_By(var self, bool(*f)(var,var)) {
  Tuple_Sort_Part(self, 0, Tuple_Len(self)-1, f);
}

static int Tuple_Cmp(var self, var obj) {
  struct Tuple* t = self;
  
  size_t n = Tuple_Len(self);
  int c = n - len(obj);
  if (c isnt 0) { return c; }
  
  for (size_t i = 0; i < n; i++) {
    c = cmp(t->items[i], get(obj, $I(i)));
    if (c < 0) { return -1; }
    if (c > 0) { return  1; }
  }
  
  return 0;
}

static uint64_t Tuple_Hash(var self) {
  struct Tuple* t = self;
  uint64_t h = 0;
  
  size_t n = Tuple_Len(self);
  for (size_t i = 0; i < n; i++) {
    h ^= hash(t->items[i]);
  }
  
  return h;
}

static var Tuple_Subtype(var self) {
  return Ref;
}

var Tuple = Cello(Tuple,
  Instance(Doc,
    Tuple_Name,       Tuple_Brief,    Tuple_Description, 
    Tuple_Definition, Tuple_Examples, Tuple_Methods),
  Instance(New,      Tuple_New, Tuple_Del),
  Instance(Assign,   Tuple_Assign),
  Instance(Cmp,      Tuple_Cmp),
  Instance(Hash,     Tuple_Hash),
  Instance(Len,      Tuple_Len),
  Instance(Get,      Tuple_Get, Tuple_Set, Tuple_Mem, Tuple_Rem),
  Instance(Push,     Tuple_Push, Tuple_Pop, Tuple_Push_At, Tuple_Pop_At),
  Instance(Concat,   Tuple_Concat, Tuple_Push),
  Instance(Clear,    Tuple_Clear),
  Instance(Subtype,  Tuple_Subtype),
  Instance(Iter, 
    Tuple_Iter_Init, Tuple_Iter_Next, 
    Tuple_Iter_Last, Tuple_Iter_Prev),
  Instance(Mark,     Tuple_Mark),
  Instance(Reverse,  Tuple_Reverse),
  Instance(Sort,     Tuple_Sort_By),
  Instance(Show,     Tuple_Show, NULL));

