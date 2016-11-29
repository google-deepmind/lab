// Copyright (C) 2016 Google Inc.
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

static void LabObject_dealloc(LabObject* self) {
  env_close(self);
  free(self->env_c_api);
  free(self->observation_indices);
  self->ob_type->tp_free((PyObject*)self);
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

    DeepMindLabLaunchParams params;
    params.runfiles_path = runfiles_path;

    if (dmlab_connect(&params, self->env_c_api, &self->context) != 0) {
      PyErr_SetString(PyExc_RuntimeError, "Failed to connect RL API");
      free(self->env_c_api);
      return NULL;
    }

    if (self->env_c_api->setting(self->context, "actionSpec", "Integers")
        != 0) {
      PyErr_SetString(PyExc_RuntimeError,
                      "Failed to apply 'actionSpec' setting.");
      free(self->env_c_api);
      return NULL;
    }

    if (self->context == NULL) {
      Py_DECREF(self);
      free(self->env_c_api);
      return NULL;
    }

    self->status = ENV_STATUS_UNINITIALIZED;
    self->episode = 0;
  }

  return (PyObject*)self;
}

static int Lab_init(LabObject* self, PyObject* args, PyObject* kwds) {
  char* level;
  PyObject *observations = NULL, *config = NULL;

  static char* kwlist[] = {"level", "observations", "config", NULL};

  if (self->env_c_api == NULL) {
    PyErr_SetString(PyExc_RuntimeError, "RL API not setup");
    return -1;
  }

  if (!PyArg_ParseTupleAndKeywords(args, kwds, "sO!|O!", kwlist, &level,
                                   &PyList_Type, &observations, &PyDict_Type,
                                   &config)) {
    return -1;
  }

  if (self->env_c_api->setting(self->context, "levelName", level) != 0) {
    PyErr_Format(PyExc_RuntimeError, "Invalid levelName flag '%s'", level);
    return -1;
  }

  self->observation_count = PyList_Size(observations);
  self->observation_indices = calloc(self->observation_count, sizeof(int));
  if (self->observation_indices == NULL) {
    PyErr_NoMemory();
    return -1;
  }

  if (self->env_c_api->setting(self->context, "fps", "60") != 0) {
    PyErr_SetString(PyExc_RuntimeError, "Failed to set fps");
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
        PyErr_Format(PyExc_RuntimeError, "Failed to apply setting '%s = %s'.",
                     key, value);
      }
    }
  }

  if (self->env_c_api->init(self->context) != 0) {
    PyErr_Format(PyExc_RuntimeError, "Failed to init environment.");
    return -1;
  }

  char* observation_name;
  for (int i = 0; i < self->observation_count; ++i) {
    observation_name = PyString_AsString(PyList_GetItem(observations, i));
    if (observation_name == NULL) {
      return -1;
    }
    int j;
    for (j = 0; j < self->env_c_api->observation_count(self->context); ++j) {
      if (strcmp(self->env_c_api->observation_name(self->context, j),
                 observation_name) == 0) {
        self->observation_indices[i] = j;
        break;
      }
    }
    if (j == self->env_c_api->observation_count(self->context)) {
      PyErr_Format(PyExc_ValueError, "Unknown observation '%s'.",
                   observation_name);
      return -1;
    }
  }

  return 0;
}

static PyObject* Lab_reset(LabObject* self, PyObject* args, PyObject* kwds) {
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
    PyErr_SetString(PyExc_RuntimeError, "Failed to start environment.");
    return NULL;
  }
  self->num_steps = 0;
  ++self->episode;
  self->status = ENV_STATUS_INITIALIZED;
  Py_RETURN_TRUE;
}

static PyObject* Lab_num_steps(LabObject* self) {
  return PyInt_FromLong(self->num_steps);
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

static PyObject* Lab_is_running(LabObject* self) {
  if (is_running(self)) {
    Py_RETURN_TRUE;
  } else {
    Py_RETURN_FALSE;
  }
}

static PyObject* Lab_step(LabObject* self, PyObject* args, PyObject* kwds) {
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

  if (PyArray_NDIM(discrete) != 1 ||
      PyArray_DIM(discrete, 0) !=
          self->env_c_api->action_discrete_count(self->context)) {
    PyErr_Format(PyExc_ValueError, "action must have shape (%i)",
                 self->env_c_api->action_discrete_count(self->context));
    return NULL;
  }

  if (PyArray_TYPE(discrete) != NPY_INT) {
    PyErr_SetString(PyExc_ValueError, "action must have dtype np.intc");
    return NULL;
  }

  self->env_c_api->act(self->context, (int*)PyArray_DATA(discrete), NULL);

  self->status = self->env_c_api->advance(self->context, num_steps, &reward);
  self->num_steps += num_steps;
  return PyFloat_FromDouble(reward);
}

// Helper function to convert our types into Numpy types
static int ObservationType2typenum(EnvCApi_ObservationType type) {
  switch (type) {
    case EnvCApi_ObservationDoubles:
      return NPY_DOUBLE;
    case EnvCApi_ObservationBytes:
      return NPY_UINT8;
    default:
      return -1;
  }
}

static PyObject* Lab_observation_spec(LabObject* self) {
  int count = self->env_c_api->observation_count(self->context);
  PyObject* result = PyList_New(count);
  if (result == NULL) {
    PyErr_NoMemory();
    return NULL;
  }
  EnvCApi_ObservationSpec spec;
  PyObject* type;
  PyObject* shape;

  for (int i = 0; i < count; ++i) {
    self->env_c_api->observation_spec(self->context, i, &spec);
    type = (PyObject*)PyArray_DescrFromType(ObservationType2typenum(spec.type))
               ->typeobj;
    shape = PyTuple_New(spec.dims);
    for (int j = 0; j < spec.dims; ++j) {
      if (PyTuple_SetItem(shape, j, PyInt_FromLong(spec.shape[j])) != 0) {
        PyErr_SetString(PyExc_RuntimeError, "Unable to populate tuple");
        return NULL;
      }
    }
    if (PyList_SetItem(
            result, i,
            Py_BuildValue("{s:s,s:N,s:O}", "name",
                          self->env_c_api->observation_name(self->context, i),
                          "shape", shape, "dtype", type)) != 0) {
      PyErr_SetString(PyExc_RuntimeError, "Unable to populate list");
      return NULL;
    }
  }
  return result;
}

static PyObject* Lab_fps(LabObject* self) {
  return PyInt_FromLong(self->env_c_api->fps(self->context));
}

static PyObject* Lab_action_spec(LabObject* self) {
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

static PyObject* Lab_observations(LabObject* self) {
  PyObject* result = NULL;
  PyArrayObject* array = NULL;

  if (!is_running(self)) {
    PyErr_SetString(PyExc_RuntimeError,
                    "Environment in wrong status for call to observations()");
    return NULL;
  }

  result = PyDict_New();
  if (result == NULL) {
    PyErr_NoMemory();
    return NULL;
  }

  EnvCApi_Observation observation;
  long* bounds = NULL;

  for (int i = 0; i < self->observation_count; ++i) {
    self->env_c_api->observation(self->context, self->observation_indices[i],
                                 &observation);
    bounds = calloc(observation.spec.dims, sizeof(long));
    if (bounds == NULL) {
      PyErr_NoMemory();
      return NULL;
    }
    for (int j = 0; j < observation.spec.dims; ++j) {
      bounds[j] = observation.spec.shape[j];
    }
    const void* src_mem = observation.spec.type == EnvCApi_ObservationDoubles
                              ? (void*)observation.payload.doubles
                              : (void*)observation.payload.bytes;
    array = (PyArrayObject*)PyArray_SimpleNew(
        observation.spec.dims, bounds,
        ObservationType2typenum(observation.spec.type));
    free(bounds);

    if (array == NULL) {
      PyErr_NoMemory();
      return NULL;
    }

    memcpy(PyArray_BYTES(array), src_mem, PyArray_NBYTES(array));
    PyDict_SetItemString(result,
                         self->env_c_api->observation_name(
                             self->context, self->observation_indices[i]),
                         (PyObject*)array);
    Py_DECREF((PyObject*)array);
  }

  return result;
}

static PyObject* Lab_close(LabObject* self) {
  if (env_close(self)) {
    Py_RETURN_TRUE;
  } else {
    Py_RETURN_FALSE;
  }
}

static PyMethodDef LabObject_methods[] = {
    {"reset", (PyCFunction)Lab_reset, METH_VARARGS | METH_KEYWORDS,
     "Reset the environment"},
    {"num_steps", (PyCFunction)Lab_num_steps, METH_NOARGS,
     "Number of frames since the last reset() call"},
    {"is_running", (PyCFunction)Lab_is_running, METH_NOARGS,
     "If the environment is in status RUNNING"},
    {"step", (PyCFunction)Lab_step, METH_VARARGS | METH_KEYWORDS,
     "Advance the environment a number of steps"},
    {"observation_spec", (PyCFunction)Lab_observation_spec, METH_NOARGS,
     "The shape of the observations"},
    {"fps", (PyCFunction)Lab_fps, METH_NOARGS,
     "An advisory metric that correlates discrete environment steps "
     "(\"frames\") with real (wallclock) time: the number of frames per (real) "
     "second."},
    {"action_spec", (PyCFunction)Lab_action_spec, METH_NOARGS,
     "The shape of the actions"},
    {"observations", (PyCFunction)Lab_observations, METH_NOARGS,
     "Get the observations"},
    {"close", (PyCFunction)Lab_close, METH_NOARGS, "Close the environment"},
    {NULL} /* Sentinel */
};

static PyTypeObject deepmind_lab_LabType = {
    PyObject_HEAD_INIT(NULL) 0,    /* ob_size */
    "deepmind_lab.Lab",            /* tp_name */
    sizeof(LabObject),             /* tp_basicsize */
    0,                             /* tp_itemsize */
    (destructor)LabObject_dealloc, /* tp_dealloc */
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
    (initproc)Lab_init,            /* tp_init */
    0,                             /* tp_alloc */
    LabObject_new,                 /* tp_new */
};

static PyObject* module_runfiles_path(PyObject* self) {
  return Py_BuildValue("s", runfiles_path);
}

static PyObject* module_set_runfiles_path(PyObject* self, PyObject* args) {
  const char* new_path;
  if (!PyArg_ParseTuple(args, "s", &new_path)) {
    return NULL;
  }
  if (sizeof(runfiles_path) < strlen(new_path)) {
    PyErr_SetString(PyExc_RuntimeError, "Runfiles directory name too long!");
    return NULL;
  }
  strcpy(runfiles_path, new_path);
  Py_RETURN_TRUE;
}

static PyObject* module_version(PyObject* self) {
  return Py_BuildValue("s", DEEPMIND_LAB_WRAPPER_VERSION);
}

static PyMethodDef module_methods[] = {
    {"version", (PyCFunction)module_version, METH_NOARGS,
     "Module version number."},
    {"runfiles_path", (PyCFunction)module_runfiles_path, METH_NOARGS,
     "Get the module-wide runfiles path."},
    {"set_runfiles_path", (PyCFunction)module_set_runfiles_path, METH_VARARGS,
     "Set the module-wide runfiles path."},
    {NULL, NULL, 0, NULL} /* sentinel */
};

PyMODINIT_FUNC initdeepmind_lab(void) {
  PyObject* m;

  if (PyType_Ready(&deepmind_lab_LabType) < 0) return;

  m = Py_InitModule3("deepmind_lab", module_methods, "DeepMind Lab API module");

  Py_INCREF(&deepmind_lab_LabType);
  PyModule_AddObject(m, "Lab", (PyObject*)&deepmind_lab_LabType);

#ifdef DEEPMIND_LAB_MODULE_RUNFILES_DIR
  static const char kRunfiles[] = ".";
#else
  static const char kRunfiles[] = ".runfiles/org_deepmind_lab";
  if (sizeof(runfiles_path) <
      strlen(Py_GetProgramFullPath()) + sizeof(kRunfiles)) {
    PyErr_SetString(PyExc_RuntimeError, "Runfiles directory name too long!");
    return;
  }
  strcpy(runfiles_path, Py_GetProgramFullPath());
#endif
  strcat(runfiles_path, kRunfiles);

  srand(time(NULL));

  import_array();
}
