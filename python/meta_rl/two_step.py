import threading
import multiprocessing
import numpy as np
import matplotlib.pyplot as plt
import tensorflow as tf
import tensorflow.contrib.slim as slim
import scipy.signal
import os
from datetime import datetime
from meta_rl.ac_network import AC_Network
from meta_rl.worker import Worker
from meta_rl.two_step_task import two_step_task
from datetime import datetime

def main():

  dir_name = "train_" + datetime.now().strftime("%m%d-%H%M%S")

  # Hyperparameters for training/testing
  gamma = .9
  a_size = 2
  n_seeds = 1
  num_episode_train = 20000
  num_episode_test = 50

  collect_seed_transition_probs = []

  # Do train and test for n_seeds different seeds
  for seed_nb in range(n_seeds):

    # initialize the directories' names to save the models for this particular seed
    model_path = dir_name+'/model_' + str(seed_nb)
    frame_path = dir_name+'/frames_' + str(seed_nb)
    plot_path = dir_name+'/plots_' + str(seed_nb)
    load_model_path = "meta_rl/results/biorxiv/final/model_" + str(seed_nb) + "/model-20000"

    # create the directories
    if not os.path.exists(model_path):
      os.makedirs(model_path)

    if not os.path.exists(frame_path):
      os.makedirs(frame_path)

    if not os.path.exists(plot_path):
      os.makedirs(plot_path)

    # in train don't load the model and set train=True
    # in test, load the model and set train=False
    for train, load_model, num_episodes in [[False, True, num_episode_test]]:#[[True,False,num_episode_train], [False, True, num_episode_test]]:

      print ("seed_nb is:", seed_nb)

      # resets tensorflow graph between train/test and seeds to avoid clutter
      tf.reset_default_graph()

      with tf.device("/cpu:0"):
        global_episodes = tf.Variable(0,dtype=tf.int32,name='global_episodes',trainable=False)
        trainer = tf.train.RMSPropOptimizer(learning_rate=7e-4)
        master_network = AC_Network(a_size,'global',None) # Generate global network
        num_workers = 1
        workers = []
        # Create worker classes
        for i in range(num_workers):
          workers.append(Worker(two_step_task(),i,a_size,trainer,model_path,global_episodes, make_gif=True, collect_seed_transition_probs=collect_seed_transition_probs, plot_path=plot_path, frame_path=frame_path))
        saver = tf.train.Saver(max_to_keep=5)

      with tf.Session() as sess:
        # set the seed
        np.random.seed(seed_nb)
        tf.set_random_seed(seed_nb)

        coord = tf.train.Coordinator()
        if load_model == True:
          print ('Loading Model...')
          ckpt = tf.train.get_checkpoint_state(load_model_path)
          saver.restore(sess,ckpt.model_checkpoint_path)
        else:
          sess.run(tf.global_variables_initializer())

        worker_threads = []
        for worker in workers:
          worker_work = lambda: worker.work(gamma,sess,coord,saver,train,num_episodes)
          thread = threading.Thread(target=(worker_work))
          thread.start()
          worker_threads.append(thread)
        coord.join(worker_threads)

  # final plot of the different seeds

  common_sum = np.array([0.,0.])
  uncommon_sum = np.array([0.,0.])

  fig, ax = plt.subplots()

  for i in range(n_seeds):

    x = np.arange(2)
    ax.set_ylim([0.5, 1.0])
    ax.set_ylabel('Stay Probability')

    common, uncommon = collect_seed_transition_probs[i]

    common_sum += np.array(common)
    uncommon_sum += np.array(uncommon)

    ax.set_xticks([1.3,3.3])
    ax.set_xticklabels(['Last trial rewarded', 'Last trial not rewarded'])

    plt.plot([1,3], common, 'o', color='black');
    plt.plot([1.8,3.8], uncommon, 'o', color='black');

  c = plt.bar([1.,3.],  (1. / n_seeds) * common_sum, color='b', width=0.5)
  uc = plt.bar([1.8,3.8], (1. / n_seeds) * uncommon_sum, color='r', width=0.5)
  ax.legend( (c[0], uc[0]), ('common', 'uncommon') )
  plt.savefig(dir_name +"/final_plot.png")

  plt.show()

if __name__ == '__main__':
  main()
