// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PPAPI_C_PPB_VAR_H_
#define PPAPI_C_PPB_VAR_H_

#include "ppapi/c/pp_stdint.h"
#include "ppapi/c/pp_var.h"

typedef struct _ppp_Class PPP_Class;

#define PPB_VAR_INTERFACE "PPB_Var;1"

// Please see:
//   http://code.google.com/p/ppapi/wiki/InterfacingWithJavaScript
// for general information on using this interface.
typedef struct _ppb_Var {
  // Adds a reference to the given var. If this is not a refcounted object,
  // this function will do nothing so you can always call it no matter what the
  // type.
  void (*AddRef)(PP_Var var);

  // Removes a reference to given var, deleting it if the internal refcount
  // becomes 0. If the given var is not a refcounted object, this function will
  // do nothing so you can always call it no matter what the type.
  void (*Release)(PP_Var var);

  // Creates a string var from a string. The string is encoded in UTF-8 and is
  // NOT NULL-terminated, the length must be specified in |len|. If the length
  // is 0, the |data| pointer will not be dereferenced and may be NULL. Note,
  // however, that if you do this, the "NULL-ness" will not be preserved, as
  // VarToUtf8 will never return NULL on success, even for empty strings.
  //
  // The resulting object will be a refcounted string object. It will be
  // AddRef()ed for the caller. When the caller is done with it, it should be
  // Release()d.
  //
  // On error (basically out of memory to allocate the string) this function
  // will return a Void var.
  PP_Var (*VarFromUtf8)(const char* data, uint32_t len);

  // Converts a string-type var to a char* encoded in UTF-8. This string is NOT
  // NULL-terminated. The length will be placed in |*len|. If the string is
  // valid but empty, the return value will be non-NULL, but |*len| will still
  // be 0.
  //
  // If the var is not a string, this function will return NULL and |*len| will
  // be 0. Note that if the Var is corrupt or the string has been freed, this
  // function may crash, it is the plugin's responsibility to manage the memory
  // properly.
  const char* (*VarToUtf8)(PP_Var var, uint32_t* len);

  // Returns true if the property with the given name exists on the given
  // object, false if it does not. Methods are also counted as properties.
  //
  // The name can either be a string or an integer var. It is an error to pass
  // another type of var as the name.
  //
  // If you pass an invalid name or object, the exception will be set (if it is
  // non-NULL, and the return value will be false).
  bool (*HasProperty)(PP_Var object,
                      PP_Var name,
                      PP_Var* exception);

  // Identical to HasProperty, except that HasMethod additionally checks if the
  // property is a function.
  bool (*HasMethod)(PP_Var object,
                    PP_Var name,
                    PP_Var* exception);

  // Returns the value of the given property. If the property doesn't exist, the
  // exception (if non-NULL) will be set and a "Void" var will be returned.
  PP_Var (*GetProperty)(PP_Var object,
                        PP_Var name,
                        PP_Var* exception);

  // Retrieves all property names on the given object. Property names include
  // methods.
  //
  // If there is a failure, the given exception will be set (if it is non-NULL).
  // On failure, |*properties| will be set to NULL and |*property_count| will be
  // set to 0.
  //
  // A pointer to the array of property names will be placesd in |*properties|.
  // The caller is responsible for calling Release() on each of these properties
  // (as per normal refcounted memory management) as well as freeing the array
  // pointer with PPB_Core.MemFree().
  //
  // This function returns all "enumerable" properties. Some JavaScript
  // properties are "hidden" and these properties won't be retrieved by this
  // function, yet you can still set and get them.
  //
  // Example:
  //   uint32_t count;
  //   PP_Var* properties;
  //   ppb_var.GetAllPropertyNames(object, &count, &properties);
  //
  //   ...use the properties here...
  //
  //   for (uint32_t i = 0; i < count; i++)
  //     ppb_var.Release(properties[i]);
  //   ppb_core.MemFree(properties);
  void (*GetAllPropertyNames)(PP_Var object,
                              uint32_t* property_count,
                              PP_Var** properties,
                              PP_Var* exception);

  // Sets the property with the given name on the given object. The exception
  // will be set, if it is non-NULL, on failure.
  void (*SetProperty)(PP_Var object,
                      PP_Var name,
                      PP_Var value,
                      PP_Var* exception);

  // Removes the given property from the given object. The property name must
  // be an string or integer var, using other types will throw an exception
  // (assuming the exception pointer is non-NULL).
  void (*RemoveProperty)(PP_Var object,
                         PP_Var name,
                         PP_Var* exception);

  // TODO(brettw) need native array access here.

  // Invoke the function |method_name| on the given object. If |method_name|
  // is a Null var, the default method will be invoked, which is how you can
  // invoke function objects.
  //
  // Unless it is type Null, |method_name| must be a string. Unlike other
  // Var functions, integer lookup is not supported since you can't call
  // functions on integers in JavaScript.
  //
  // Pass the arguments to the function in order in the |argv| array, and the
  // number of arguments in the |argc| parameter. |argv| can be NULL if |argc|
  // is zero.
  //
  // Example:
  //   Call(obj, VarFromUtf8("DoIt"), 0, NULL, NULL) = obj.DoIt() in JavaScript.
  //   Call(obj, PP_MakeNull(), 0, NULL, NULL) = obj() in JavaScript.
  PP_Var (*Call)(PP_Var object,
                 PP_Var method_name,
                 uint32_t argc,
                 PP_Var* argv,
                 PP_Var* exception);

  // Invoke the object as a constructor.
  //
  // For example, if |object| is |String|, this is like saying |new String| in
  // JavaScript.
  PP_Var (*Construct)(PP_Var object,
                      uint32_t argc,
                      PP_Var* argv,
                      PP_Var* exception);

  // If the object is an instance of the given class, then this method returns
  // true and sets *object_data to the value passed to CreateObject provided
  // object_data is non-NULL. Otherwise, this method returns false.
  bool (*IsInstanceOf)(PP_Var var,
                       const PPP_Class* object_class,
                       void** object_data);

  // Creates an object that the plugin implements. The plugin supplies a
  // pointer to the class interface it implements for that object, and its
  // associated internal data that represents that object.
  //
  // The returned object will have a reference count of 1. When the reference
  // count reached 0, the class' Destruct function wlil be called.
  //
  // Example: Say we're implementing a "Point" object.
  //   void PointDestruct(void* object) {
  //     delete (Point*)object;
  //   }
  //
  //   const PPP_Class point_class = {
  //     ... all the other class functions go here ...
  //     &PointDestruct
  //   };
  //
  //   // The plugin's internal object associated with the point.
  //   class Point {
  //     ...
  //   };
  //
  //   PP_Var MakePoint(int x, int y) {
  //     return CreateObject(&point_class, new Point(x, y));
  //   }
  PP_Var (*CreateObject)(const PPP_Class* object_class,
                         void* object_data);
} PPB_Var;

#endif  // PPAPI_C_PPB_VAR_H_