<?php

/**
 * @file
 * Provide new views field and handlers for user.
 */

/**
 * Implements hook_views_data().
 */
function users_online_views_data() {

  $data['users_online']['table']['group'] = t('Usersonline');
  $data['users_online']['table']['join'] = array(
    // #global is a special flag which let's a table appear all the time.
    '#global' => array(),
  );
 
  $data['users_online']['is_user_online'] = array(
    'title' => t('is user online'),
    'help' => t('Determine if a user is or not online".'),
    'field' => array(
      'handler' => 'views_handler_usersonline_field',
    ),
   // 'group' => 'usersonline',
  );
 
  return $data;
}