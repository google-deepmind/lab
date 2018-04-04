// Copyright (C) 2016-2017 Google Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
////////////////////////////////////////////////////////////////////////////////
//
// Python wrapper module for exposing the DeepMind Lab environment.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <Python.h>
// Disallow Numpy 1.7 deprecated symbols.
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include "numpy/arrayobject.h"

#include "public/dmlab.h"

#define DEEPMIND_LAB_WRAPPER_VERSION "1.0"

#define ENV_STATUS_CLOSED -3
#define ENV_STATUS_UNINITIALIZED -2
#define ENV_STATUS_INITIALIZED -1


// Glue code to make the code work with both Python 2.7 and Python 3.
//
#if PY_MAJOR_VERSION >= 3
#  define PY_INIT_SIGNATURE(name) PyInit_ ## name
#  define PY_RETURN_SUCCESS(x) return x
#  define PY_RETURN_FAILURE    return NULL
#  define PyInt_FromLong PyLong_FromLong
#  define PyInt_AsLong PyLong_AsLong
#  define PyInt_Check PyLong_Check
#  define PyString_FromString PyBytes_FromString
#  define PyString_FromStringAndSize PyBytes_FromStringAndSize
#  define PyString_AsString PyBytes_AsString
#  define PyString_Check PyBytes_Check
#  define PyString_Type PyBytes_Type
#else  // PY_MAJOR_VERSION >= 3
#  define PY_INIT_SIGNATURE(name) init ## name
#  define PY_RETURN_SUCCESS(x) return
#  define PY_RETURN_FAILURE    return
#endif  // PY_MAJOR_VERSION >= 3

static char runfiles_path[4096];  // set in initdeepmind_lab() below

typedef struct {
  PyObject_HEAD
  EnvCApi* env_c_api;
  void* context;
  int status;
  int episode;
  int* observation_indices;
  int observation_count;
  int num_steps;
} LabObject;

// Helper function to close the environment.
static int env_close(LabObject* self) {
  if (self->status != ENV_STATUS_CLOSED) {
    self->env_c_api->release_context(self->context);
    self->status = ENV_STATUS_CLOSED;
    return true;
  }
  return false;
}

static void LabObject_dealloc(PyObject* pself) {
  LabObject* self = (LabObject*)pself;
  env_close(self);
  free(self->env_c_api);
  free(self->observation_indices);
  Py_TYPE(self)->tp_free(pself);
}

static PyObject* LabObject_new(PyTypeObject* type, PyObject* args,
                               PyObject* kwds) {
  LabObject* self;

  self = (LabObject*)type->tp_alloc(type, 0);
  if (self != NULL) {
    self->env_c_api = calloc(1, sizeof *self->env_c_api);

    if (self->env_c_api == NULL) {
      PyErr_SetString(PyExc_MemoryError, "malloc failed.");
      return NULL;
    }
    self->status = ENV_STATUS_CLOSED;
  }
  return (PyObject*)self;
}

static int Lab_init(PyObject* pself, PyObject* args, PyObject* kwds) {
  LabObject* self = (LabObject*)pself;
  char* level;
  char* renderer = NULL;
  PyObject *observations = NULL, *config = NULL;

  static char* kwlist[] = {"level", "observations", "config", "renderer", NULL};

  if (self->env_c_api == NULL) {
    PyErr_SetString(PyExc_RuntimeError, "RL API not setup");
    return -1;
  }

  if (!PyArg_ParseTupleAndKeywords(args, kwds, "sO!|O!s", kwlist, &level,
                                   &PyList_Type, &observations, &PyDict_Type,
                                   &config, &renderer)) {
    return -1;
  }

  self->observation_count = PyList_Size(observations);
  self->observation_indices = calloc(self->observation_count, sizeof(int));
  if (self->observation_indices == NULL) {
    PyErr_NoMemory();
    return -1;
  }
  {
    DeepMindLabLaunchParams params = {};
    params.runfiles_path = runfiles_path;
    params.renderer = DeepMindLabRenderer_Software;
    if (renderer != NULL && renderer[0] != '\0') {
      if (strcmp(renderer, "hardware") == 0) {
        params.renderer = DeepMindLabRenderer_Hardware;
      } else if (strcmp(renderer, "software") != 0) {
        PyErr_Format(PyExc_RuntimeError,
                     "Failed to set renderer must be \"hardware\" or "
                     "\"software\" actual \"%s\"!",
                     renderer);
        return -1;
      }
    }

    if (dmlab_connect(&params, self->env_c_api, &self->context) != 0) {
      PyErr_SetString(PyExc_RuntimeError, "Failed to connect RL API");
      return -1;
    }

// When running under TSAN, switch to the interpreted VM, which is
// instrumentable.
//
// It might be a better idea add __attribute__((no_sanitize("thread"))) to
// vm_x86.c, but I have not managed to make that work.
#ifndef __has_feature
#  define __has_feature(x) 0
#endif
#if __has_feature(thread_sanitizer)
    if (self->env_c_api->setting(self->context, "vmMode", "interpreted") != 0) {
      PyErr_Format(PyExc_RuntimeError,
                   "Failed to apply 'vmMode' setting - \"%s\"",
                   self->env_c_api->error_message(self->context));
      return -1;
    }
#endif
    self->status = ENV_STATUS_UNINITIALIZED;
    self->episode = 0;
  }

  if (self->env_c_api->setting(self->context, "levelName", level) != 0) {
    PyErr_Format(PyExc_RuntimeError, "Invalid levelName flag '%s' - \"%s\"",
                 level, self->env_c_api->error_message(self->context));
    return -1;
  }

  if (self->env_c_api->setting(self->context, "fps", "60") != 0) {
    PyErr_Format(PyExc_RuntimeError, "Failed to set fps - \"%s\"",
                 self->env_c_api->error_message(self->context));
    return -1;
  }

  if (config != NULL) {
    PyObject *pykey, *pyvalue;
    Py_ssize_t pos = 0;
    char *key, *value;

    while (PyDict_Next(config, &pos, &pykey, &pyvalue)) {
      key = PyString_AsString(pykey);
      value = PyString_AsString(pyvalue);
      if (key == NULL || value == NULL) {
        return -1;
      }
      if (self->env_c_api->setting(self->context, key, value) != 0) {
        PyErr_Format(PyExc_RuntimeError,
                     "Failed to apply setting '%s = %s' - \"%s\"", key, value,
                     self->env_c_api->error_message(self->context));
        return -1;
      }
    }
  }

  if (self->env_c_api->init(self->context) != 0) {
    PyErr_Format(PyExc_RuntimeError, "Failed to init environment - \"%s\"",
                 self->env_c_api->error_message(self->context));
    return -1;
  }

  char* observation_name;
  int api_observation_count = self->env_c_api->observation_count(self->context);
  for (int i = 0; i < self->observation_count; ++i) {
    observation_name = PyString_AsString(PyList_GetItem(observations, i));
    if (observation_name == NULL) {
      return -1;
    }
    int j;
    for (j = 0; j < api_observation_count; ++j) {
      if (strcmp(self->env_c_api->observation_name(self->context, j),
                 observation_name) == 0) {
        self->observation_indices[i] = j;
        break;
      }
    }
    if (j == api_observation_count) {
      PyErr_Format(PyExc_ValueError, "Unknown observation '%s'.",
                   observation_name);
      return -1;
    }
  }

  return 0;
}

static PyObject* Lab_reset(PyObject* pself, PyObject* args, PyObject* kwds) {
  LabObject* self = (LabObject*)pself;
  int episode = -1;
  int seed;
  PyObject* seed_arg = NULL;

  char* kwlist[] = {"episode", "seed", NULL};
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "|iO", kwlist, &episode,
                                   &seed_arg)) {
    return NULL;
  }

  if (episode >= 0) {
    self->episode = episode;
  }

  if (seed_arg == NULL || seed_arg == Py_None) {
    seed = rand();
  } else {
    if (!PyInt_Check(seed_arg)) {
      PyErr_Format(PyExc_ValueError, "'seed' must be int or None, was '%s'.",
                   Py_TYPE(seed_arg)->tp_name);
      return NULL;
    }
    seed = PyInt_AsLong(seed_arg);
  }

  if (self->env_c_api->start(self->context, self->episode, seed) != 0) {
    PyErr_Format(PyExc_RuntimeError, "Failed to start environment - \"%s\"",
                 self->env_c_api->error_message(self->context));
    return NULL;
  }
  self->num_steps = 0;
  ++self->episode;
  self->status = ENV_STATUS_INITIALIZED;
  Py_RETURN_TRUE;
}

static PyObject* Lab_num_steps(PyObject* self, PyObject* no_arg) {
  return PyInt_FromLong(((LabObject*)self)->num_steps);
}

// Helper function to determine if we're ready to step or give an observation.
static int is_running(LabObject* self) {
  switch (self->status) {
    case ENV_STATUS_INITIALIZED:
    case EnvCApi_EnvironmentStatus_Running:
      return true;
    default:
      return false;
  }
}

static PyObject* Lab_is_running(PyObject* self, PyObject* no_arg) {
  if (is_running((LabObject*)self)) {
    Py_RETURN_TRUE;
  } else {
    Py_RETURN_FALSE;
  }
}

static PyObject* Lab_step(PyObject* pself, PyObject* args, PyObject* kwds) {
  LabObject* self = (LabObject*)pself;
  PyObject* action_obj = NULL;
  int num_steps = 1;

  char* kwlist[] = {"action", "num_steps", NULL};
  double reward;

  if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!|i", kwlist, &PyArray_Type,
                                   &action_obj, &num_steps)) {
    return NULL;
  }

  if (!is_running(self)) {
    PyErr_SetString(PyExc_RuntimeError,
                    "Environment in wrong status to advance.");
    return NULL;
  }

  PyArrayObject* discrete = (PyArrayObject*)action_obj;

  int action_discrete_count =
      self->env_c_api->action_discrete_count(self->context);
  if (PyArray_NDIM(discrete) != 1 ||
      PyArray_DIM(discrete, 0) != action_discrete_count) {
    PyErr_Format(PyExc_ValueError, "action must have shape (%i)",
                 action_discrete_count);
    return NULL;
  }

  if (PyArray_TYPE(discrete) != NPY_INT) {
    PyErr_SetString(PyExc_ValueError, "action must have dtype np.intc");
    return NULL;
  }

  self->env_c_api->act(self->context, (int*)PyArray_DATA(discrete), NULL);

  self->status = self->env_c_api->advance(self->context, num_steps, &reward);
  self->num_steps += num_steps;
  if (self->status == EnvCApi_EnvironmentStatus_Error) {
    PyErr_Format(PyExc_ValueError, "Failed to advance environment \"%s\"",
                 self->env_c_api->error_message(self->context));
    return NULL;
  }
  return PyFloat_FromDouble(reward);
}

// Helper function to convert our types into Numpy types
static int ObservationType2typenum(EnvCApi_ObservationType type) {
  switch (type) {
    case EnvCApi_ObservationDoubles:
      return NPY_DOUBLE;
    case EnvCApi_ObservationBytes:
      return NPY_UINT8;
    case EnvCApi_ObservationString:
      return -1;
  }
  return -1;
}

static PyObject* Lab_observation_spec(PyObject* pself, PyObject* no_arg) {
  LabObject* self = (LabObject*)pself;
  int count = self->env_c_api->observation_count(self->context);
  PyObject* result = PyList_New(count);
  if (result == NULL) {
    PyErr_NoMemory();
    return NULL;
  }
  EnvCApi_ObservationSpec spec;
  PyObject* type;
  PyObject* shape;

  for (int idx = 0; idx < count; ++idx) {
    self->env_c_api->observation_spec(self->context, idx, &spec);
    if (spec.type == EnvCApi_ObservationString) {
      type = (PyObject*)(&PyString_Type);
      shape = PyTuple_New(0);
      if (PyList_SetItem(result, idx,
                         Py_BuildValue("{s:s,s:N,s:O}", "name",
                                       self->env_c_api->observation_name(
                                           self->context, idx),
                                       "shape", shape, "dtype", type)) != 0) {
        PyErr_SetString(PyExc_RuntimeError, "Unable to populate list");
        return NULL;
      }
      continue;
    }
    int observation_type = ObservationType2typenum(spec.type);
    if (observation_type == -1) {
      PyErr_SetString(PyExc_RuntimeError, "Invalid observation spec.");
      return NULL;
    }

    type = (PyObject*)PyArray_DescrFromType(observation_type)->typeobj;
    shape = PyTuple_New(spec.dims);
    for (int j = 0; j < spec.dims; ++j) {
      if (PyTuple_SetItem(shape, j, PyInt_FromLong(spec.shape[j])) != 0) {
        PyErr_SetString(PyExc_RuntimeError, "Unable to populate tuple");
        return NULL;
      }
    }
    if (PyList_SetItem(
            result, idx,
            Py_BuildValue("{s:s,s:N,s:O}", "name",
                          self->env_c_api->observation_name(self->context, idx),
                          "shape", shape, "dtype", type)) != 0) {
      PyErr_SetString(PyExc_RuntimeError, "Unable to populate list");
      return NULL;
    }
  }
  return result;
}

static PyObject* Lab_fps(PyObject* self, PyObject* no_arg) {
  return PyInt_FromLong(
      ((LabObject*)self)->env_c_api->fps(((LabObject*)self)->context));
}

static PyObject* Lab_action_spec(PyObject* pself, PyObject* no_arg) {
  LabObject* self = (LabObject*)pself;
  PyObject* discrete;

  int count = self->env_c_api->action_discrete_count(self->context);
  discrete = PyList_New(count);
  if (discrete == NULL) {
    PyErr_NoMemory();
    return NULL;
  }
  int min_discrete, max_discrete;
  for (int i = 0; i < count; ++i) {
    self->env_c_api->action_discrete_bounds(self->context, i, &min_discrete,
                                            &max_discrete);
    if (PyList_SetItem(discrete, i,
                       Py_BuildValue("{s:i,s:i,s:s}", "min", min_discrete,
                                     "max", max_discrete, "name",
                                     self->env_c_api->action_discrete_name(
                                         self->context, i))) != 0) {
      PyErr_SetString(PyExc_RuntimeError, "Unable to populate list");
      return NULL;
    }
  }

  return discrete;
}

static PyObject* make_observation(const EnvCApi_Observation* observation) {
  if (observation->spec.type == EnvCApi_ObservationString) {
    PyObject* result = PyString_FromStringAndSize(observation->payload.string,
                                                  observation->spec.shape[0]);
    if (result == NULL) PyErr_NoMemory();
    return result;
  }

  int observation_type = ObservationType2typenum(observation->spec.type);
  if (observation_type == -1) {
    PyErr_SetString(PyExc_RuntimeError, "Invalid observation spec.");
    return NULL;
  }

  long* bounds = calloc(observation->spec.dims, sizeof(long));
  if (bounds == NULL) {
    PyErr_NoMemory();
    return NULL;
  }

  for (int j = 0; j < observation->spec.dims; ++j) {
    bounds[j] = observation->spec.shape[j];
  }

  PyArrayObject* array = (PyArrayObject*)PyArray_SimpleNew(
      observation->spec.dims, bounds, observation_type);
  free(bounds);

  if (array == NULL) {
    PyErr_NoMemory();
    return NULL;
  }

  const void* src_mem = observation->spec.type == EnvCApi_ObservationDoubles
                            ? (void*)observation->payload.doubles
                            : (void*)observation->payload.bytes;
  memcpy(PyArray_BYTES(array), src_mem, PyArray_NBYTES(array));
  return (PyObject*)array;
}

static PyObject* Lab_observations(PyObject* pself, PyObject* no_arg) {
  LabObject* self = (LabObject*)pself;

  if (!is_running((self))) {
    PyErr_SetString(PyExc_RuntimeError,
                    "Environment in wrong status for call to observations()");
    return NULL;
  }

  PyObject* result = PyDict_New();
  if (result == NULL) {
    PyErr_NoMemory();
    return NULL;
  }

  EnvCApi_Observation observation;

  for (int i = 0; i < self->observation_count; ++i) {
    self->env_c_api->observation(self->context, self->observation_indices[i],
                                 &observation);
    PyObject* entry = make_observation(&observation);
    if (entry == NULL) {
      Py_DECREF(result);
      return NULL;
    }

    // PyDict_SetItemString increments reference count.
    PyDict_SetItemString(result,
                         self->env_c_api->observation_name(
                             self->context, self->observation_indices[i]),
                         entry);
    Py_DECREF(entry);
  }
  return result;
}

static PyObject* Lab_events(PyObject* pself, PyObject* no_arg) {
  LabObject* self = (LabObject*)pself;

  switch (self->status) {
    case ENV_STATUS_INITIALIZED:
    case EnvCApi_EnvironmentStatus_Running:
    case EnvCApi_EnvironmentStatus_Terminated:
      break;
    default:
      PyErr_SetString(PyExc_RuntimeError,
                      "Environment in wrong status for call to events()");
      return NULL;
  }

  int event_type_count = self->env_c_api->event_type_count(self->context);
  int event_count = self->env_c_api->event_count(self->context);
  PyObject* result = PyList_New(event_count);
  if (result == NULL) {
    PyErr_NoMemory();
    return NULL;
  }

  for (int event_id = 0; event_id < event_count; ++event_id) {
    EnvCApi_Event event;
    self->env_c_api->event(self->context, event_id, &event);
    if (0 > event.id || event.id >= event_type_count) {
      PyErr_Format(PyExc_RuntimeError,
                   "Environment generated invalid event id. "
                   "Event id(%d) must be in range [0, %d).",
                   event.id, event_type_count);
      Py_DECREF(result);
      return NULL;
    }
    PyObject* entry = PyTuple_New(2);
    PyTuple_SetItem(entry, 0,
                    PyString_FromString(self->env_c_api->event_type_name(
                        self->context, event.id)));

    PyObject* observation_list = PyList_New(event.observation_count);
    if (observation_list == NULL) {
      Py_DECREF(result);
      return NULL;
    }
    for (int obs_id = 0; obs_id < event.observation_count; ++obs_id) {
      PyObject* obs_enty = make_observation(&event.observations[obs_id]);
      if (obs_enty == NULL) {
        Py_DECREF(observation_list);
        Py_DECREF(result);
        return NULL;
      }
      PyList_SetItem(observation_list, obs_id, obs_enty);
    }
    PyTuple_SetItem(entry, 1, observation_list);
    PyList_SetItem(result, event_id, entry);
  }

  return result;
}

static PyObject* Lab_close(PyObject* self, PyObject* no_arg) {
  if (env_close((LabObject*)self)) {
    Py_RETURN_TRUE;
  } else {
    Py_RETURN_FALSE;
  }
}

static PyMethodDef LabObject_methods[] = {
    {"reset", (PyCFunction)Lab_reset, METH_VARARGS | METH_KEYWORDS,
     "Reset the environment"},
    {"num_steps", Lab_num_steps, METH_NOARGS,
     "Number of frames since the last reset() call"},
    {"is_running", Lab_is_running, METH_NOARGS,
     "If the environment is in status RUNNING"},
    {"step", (PyCFunction)Lab_step, METH_VARARGS | METH_KEYWORDS,
     "Advance the environment a number of steps"},
    {"observation_spec", Lab_observation_spec, METH_NOARGS,
     "The shape of the observations"},
    {"fps", Lab_fps, METH_NOARGS,
     "An advisory metric that correlates discrete environment steps "
     "(\"frames\") with real (wallclock) time: the number of frames per (real) "
     "second."},
    {"action_spec", Lab_action_spec, METH_NOARGS, "The shape of the actions"},
    {"observations", Lab_observations, METH_NOARGS, "Get the observations"},
    {"events", Lab_events, METH_NOARGS, "Get the events"},
    {"close", Lab_close, METH_NOARGS, "Close the environment"},
    {NULL} /* Sentinel */
};

static PyTypeObject deepmind_lab_LabType = {
    PyVarObject_HEAD_INIT(NULL, 0) /* ob_size */
    "deepmind_lab.Lab",            /* tp_name */
    sizeof(LabObject),             /* tp_basicsize */
    0,                             /* tp_itemsize */
    LabObject_dealloc,             /* tp_dealloc */
    0,                             /* tp_print */
    0,                             /* tp_getattr */
    0,                             /* tp_setattr */
    0,                             /* tp_compare */
    0,                             /* tp_repr */
    0,                             /* tp_as_number */
    0,                             /* tp_as_sequence */
    0,                             /* tp_as_mapping */
    0,                             /* tp_hash */
    0,                             /* tp_call */
    0,                             /* tp_str */
    0,                             /* tp_getattro */
    0,                             /* tp_setattro */
    0,                             /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,            /* tp_flags */
    "Lab object",                  /* tp_doc */
    0,                             /* tp_traverse */
    0,                             /* tp_clear */
    0,                             /* tp_richcompare */
    0,                             /* tp_weaklistoffset */
    0,                             /* tp_iter */
    0,                             /* tp_iternext */
    LabObject_methods,             /* tp_methods */
    0,                             /* tp_members */
    0,                             /* tp_getset */
    0,                             /* tp_base */
    0,                             /* tp_dict */
    0,                             /* tp_descr_get */
    0,                             /* tp_descr_set */
    0,                             /* tp_dictoffset */
    Lab_init,                      /* tp_init */
    0,                             /* tp_alloc */
    LabObject_new,                 /* tp_new */
};

static PyObject* module_runfiles_path(PyObject* self, PyObject* no_arg) {
  return Py_BuildValue("s", runfiles_path);
}

static PyObject* module_set_runfiles_path(PyObject* self, PyObject* args) {
  const char* new_path;
  if (!PyArg_ParseTuple(args, "s", &new_path)) {
    return NULL;
  }

  if (strlen(new_path) < sizeof(runfiles_path)) {
    strcpy(runfiles_path, new_path);
  } else {
    PyErr_SetString(PyExc_RuntimeError, "Runfiles directory name too long!");
    return NULL;
  }

  Py_RETURN_TRUE;
}

static PyObject* module_version(PyObject* self, PyObject* no_arg) {
  return Py_BuildValue("s", DEEPMIND_LAB_WRAPPER_VERSION);
}

static PyMethodDef module_methods[] = {
    {"version", module_version, METH_NOARGS,
     "Module version number."},
    {"runfiles_path", module_runfiles_path, METH_NOARGS,
     "Get the module-wide runfiles path."},
    {"set_runfiles_path", module_set_runfiles_path, METH_VARARGS,
     "Set the module-wide runfiles path."},
    {NULL, NULL, 0, NULL} /* sentinel */
};

#if PY_MAJOR_VERSION >= 3
static PyModuleDef module_def = {
    PyModuleDef_HEAD_INIT,
    "deepmind_lab",             /* m_name */
    "DeepMind Lab API module",  /* m_doc */
    -1,                         /* m_size */
    module_methods,             /* m_methods */
    NULL,                       /* m_reload */
    NULL,                       /* m_traverse */
    NULL,                       /* m_clear */
    NULL,                       /* m_free */
};
#endif  // PY_MAJOR_VERSION >= 3

PyMODINIT_FUNC PY_INIT_SIGNATURE(deepmind_lab)(void) {
  PyObject* m;

  if (PyType_Ready(&deepmind_lab_LabType) < 0) PY_RETURN_FAILURE;

#if PY_MAJOR_VERSION >= 3
  m = PyModule_Create(&module_def);
#else  // PY_MAJOR_VERSION >= 3
  m = Py_InitModule3("deepmind_lab", module_methods, "DeepMind Lab API module");
#endif  // PY_MAJOR_VERSION >= 3

  Py_INCREF(&deepmind_lab_LabType);
  PyModule_AddObject(m, "Lab", (PyObject*)&deepmind_lab_LabType);

#ifdef DEEPMIND_LAB_MODULE_RUNFILES_DIR
  PyObject *v = PyObject_GetAttrString(m, "__file__");
  if (v && PyString_Check(v)) {
    const char* file = PyString_AsString(v);
    if (strlen(file) < sizeof(runfiles_path)) {
      strcpy(runfiles_path, file);
    } else {
      PyErr_SetString(PyExc_RuntimeError, "Runfiles directory name too long!");
      PY_RETURN_FAILURE;
    }

    char* last_slash = strrchr(runfiles_path, '/');
    if (last_slash != NULL) {
      *last_slash = '\0';
    } else {
      PyErr_SetString(PyExc_RuntimeError,
                      "Unable to determine runfiles directory!");
      PY_RETURN_FAILURE;
    }
  } else {
    strcpy(runfiles_path, ".");
  }
#else
#if PY_MAJOR_VERSION >= 3
  PyObject* u = PyUnicode_FromWideChar(Py_GetProgramFullPath(), -1);
  if (u == NULL) PY_RETURN_FAILURE;

  PyObject* p = PyUnicode_EncodeFSDefault(u);
  const char* s = PyBytes_AsString(p);
  size_t n = PyBytes_Size(p);
#else  // PY_MAJOR_VERSION >= 3
  const char* s = Py_GetProgramFullPath();
  size_t n = strlen(s);
#endif  // PY_MAJOR_VERSION >= 3
  static const char kRunfiles[] = ".runfiles/org_deepmind_lab";
  if (n + strlen(kRunfiles) < sizeof(runfiles_path)) {
    strcpy(runfiles_path, s);
    strcat(runfiles_path, kRunfiles);
  } else {
    PyErr_SetString(PyExc_RuntimeError, "Runfiles directory name too long!");
    PY_RETURN_FAILURE;
  }
#endif

  srand(time(NULL));

  import_array();

  PY_RETURN_SUCCESS(m);
}
