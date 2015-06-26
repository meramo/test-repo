<?php

function activehelper_livehelp__server_settings() {
  $output = '<h2>Server settings</h2>';

  $form = drupal_get_form('activehelper_livehelp__server_settings_form');
  $output .= render($form);

  $requests = db_query("select count(*) from {livehelp_requests}")->fetchField();
  $chat_sessions = db_query("select count(*) from {livehelp_sessions}")->fetchField();
  $messages = db_query("select count(*) from {livehelp_messages}")->fetchField();

  $output .= '<h2>General stats</h2>';

  $table_rows = array();

  $table_rows[] = array(t('Total number of request:') . ' ' . $requests);
  $table_rows[] = array(t('Total number of chat sessions:') . ' ' . $chat_sessions);
  $table_rows[] = array(t('Total number of messages:') . ' ' . $messages);

  $output .= theme_table(array(
    'header' => array(),
    'rows' => $table_rows,
    'attributes' => array('width' => '100%'),
    'sticky' => TRUE,
    'caption' => '',
    'colgroups' => array(),
    'empty' => '',
  ));

  return $output;
}

function activehelper_livehelp__server_settings_form($form, &$form_state) {
  global $activehelper_livehelp;

  $connection_timeout = 60;
  $keep_alive_timeout = 30;
  $guest_login_timeout = 60;
  $chat_refresh_rate = 6;
  $sound_alert_new_message = 1;

  $status_indicator_img_type = "gif";
  $sound_alert_new_pro_msg = 1;
  $invitation_position = "right";

  include $activehelper_livehelp['import_dir'] . '/constants.php';

  $form['connection_timeout'] = array(
    '#type' => 'textfield',
    '#title' => t('Connection Timeout (sec)'),
    '#default_value' => $connection_timeout
  );
  $form['keep_alive_timeout'] = array(
    '#type' => 'textfield',
    '#title' => t('Keep Alive Timeout (sec)'),
    '#default_value' => $keep_alive_timeout
  );
  $form['guest_login_timeout'] = array(
    '#type' => 'textfield',
    '#title' => t('Guest Login Timeout (sec)'),
    '#default_value' => $guest_login_timeout
  );
  $form['chat_refresh_rate'] = array(
    '#type' => 'textfield',
    '#title' => t('Chat Refresh Rate (sec)'),
    '#default_value' => $chat_refresh_rate
  );
  $form['sound_alert_new_message'] = array(
    '#type' => 'select',
    '#title' => t('Sound alert when a new message arrive'),
    '#options' => array(0 => t('Disable'), 1 => t('Enable')),
    '#default_value' => $sound_alert_new_message
  );

  $form['sound_alert_new_pro_msg'] = array(
    '#type' => 'select',
    '#title' => t('Sound Alert for Proactive messages'),
    '#options' => array(0 => t('Disable'), 1 => t('Enable')),
    '#default_value' => $sound_alert_new_pro_msg
  );
  $form['status_indicator_img_type'] = array(
    '#type' => 'select',
    '#title' => t('Status indicator image type'),
    '#options' => array(
      'gif' => 'gif',
      'png' => 'png',
      'jpg' => 'jpg', 
      'jpeg' => 'jpeg', 
      'bmp' => 'bmp'
    ),
    '#default_value' => $status_indicator_img_type
  );
  $form['invitation_position'] = array(
    '#type' => 'select',
    '#title' => t('Proactive messages position'),
    '#options' => array(
      'right' => t('right'),
      'center' => t('center'),
      'left' => t('left')
    ),
    '#default_value' => $invitation_position
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#weight' => 30,
    '#value' => t('Save'),
  );

  $form['reset'] = array(
    '#type' => 'submit',
    '#weight' => 32,
    '#name' => 'op__reset_settings',
    '#value' => t('Reset settings'),
  );

  $form['clear'] = array(
    '#type' => 'submit',
    '#weight' => 34,
    '#name' => 'op__clear_up_requests',
    '#value' => t('clear up requests'),
  );

  return $form;
}

function activehelper_livehelp__server_settings_form_submit($form, &$form_state) {
  if (isset($form_state['values']['op__reset_settings'])) {
    activehelper_livehelp__server_settings_reset($form, $form_state);

    return $form;
  }

  if (isset($form_state['values']['op__clear_up_requests'])) {
    activehelper_livehelp__clear_up_requests();

    return $form;
  }
 
  global $activehelper_livehelp;

  $connection_timeout = $form_state['complete form']['connection_timeout']['#default_value'];
  $keep_alive_timeout = $form_state['complete form']['keep_alive_timeout']['#default_value'];
  $guest_login_timeout= $form_state['complete form']['guest_login_timeout']['#default_value'];
  $chat_refresh_rate = $form_state['complete form']['chat_refresh_rate']['#default_value'];
  $sound_alert_new_message = $form_state['complete form']['sound_alert_new_message']['#default_value'];

  $status_indicator_img_type = $form_state['complete form']['status_indicator_img_type']['#default_value'];
  $sound_alert_new_pro_msg = $form_state['complete form']['sound_alert_new_pro_msg']['#default_value'];
  $invitation_position = $form_state['complete form']['invitation_position']['#default_value'];

  $f = fopen($activehelper_livehelp['import_dir'] . '/constants.php', 'r');
  $output = fread($f, filesize($activehelper_livehelp['import_dir'] . '/constants.php'));
  fclose($f);

  $output = str_replace('$connection_timeout = ' . $connection_timeout . ';', '$connection_timeout = ' . $form_state['values']['connection_timeout'] . ';', $output);
  $output = str_replace('$keep_alive_timeout = ' . $keep_alive_timeout . ';', '$keep_alive_timeout = ' . $form_state['values']['keep_alive_timeout'] . ';', $output);
  $output = str_replace('$guest_login_timeout= ' . $guest_login_timeout . ';', '$guest_login_timeout= ' . $form_state['values']['guest_login_timeout'] . ';', $output);
  $output = str_replace('$chat_refresh_rate = ' . $chat_refresh_rate . ';', '$chat_refresh_rate = ' . $form_state['values']['chat_refresh_rate'] . ';', $output);
  $output = str_replace('$sound_alert_new_message = ' . $sound_alert_new_message . ';', '$sound_alert_new_message = ' . $form_state['values']['sound_alert_new_message'] . ';', $output);

  $output = str_replace('$status_indicator_img_type = "' . $status_indicator_img_type . '";', '$status_indicator_img_type = "' . $form_state['values']['status_indicator_img_type'] . '";', $output);
  $output = str_replace('$sound_alert_new_pro_msg = ' . $sound_alert_new_pro_msg . ';', '$sound_alert_new_pro_msg = ' . (empty($form_state['values']['sound_alert_new_pro_msg']) ? 0 : $form_state['values']['sound_alert_new_pro_msg']) . ';', $output);
  $output = str_replace('$invitation_position = "' . $invitation_position . '";', '$invitation_position = "' . $form_state['values']['invitation_position'] . '";', $output);

  $f = fopen($activehelper_livehelp['import_dir'] . '/constants.php', 'w');
  fwrite($f, $output);
  fclose($f);

  drupal_set_message(t('The server settings have been updated.'));

  return $form;
}

function activehelper_livehelp__clear_up_requests() {
  global $activehelper_livehelp;

  $result = db_truncate('livehelp_requests')->execute();

  drupal_set_message(t('The requests were successfully cleared.'));

  drupal_goto('admin/activehelper-livehelp/server-settings');
}

function activehelper_livehelp__server_settings_reset($form, &$form_state) {
  global $activehelper_livehelp;
 
	/*
	 * Reset settings
	 */

  $connection_timeout = $form_state['complete form']['connection_timeout']['#default_value'];
  $keep_alive_timeout = $form_state['complete form']['keep_alive_timeout']['#default_value'];
  $guest_login_timeout= $form_state['complete form']['guest_login_timeout']['#default_value'];
  $chat_refresh_rate = $form_state['complete form']['chat_refresh_rate']['#default_value'];
  $sound_alert_new_message = $form_state['complete form']['sound_alert_new_message']['#default_value'];

  $status_indicator_img_type = $form_state['complete form']['status_indicator_img_type']['#default_value'];
  $sound_alert_new_pro_msg = $form_state['complete form']['sound_alert_new_pro_msg']['#default_value'];
  $invitation_position = $form_state['complete form']['invitation_position']['#default_value'];

  $f = fopen($activehelper_livehelp['import_dir'] . '/constants.php', 'r');
  $output = fread($f, filesize($activehelper_livehelp['import_dir'] . '/constants.php'));
  fclose($f);

  $output = str_replace('$connection_timeout = ' . $connection_timeout . ';', '$connection_timeout = 60;', $output);
  $output = str_replace('$keep_alive_timeout = ' . $keep_alive_timeout . ';', '$keep_alive_timeout = 30;', $output);
  $output = str_replace('$guest_login_timeout= ' . $guest_login_timeout . ';', '$guest_login_timeout= 60;', $output);
  $output = str_replace('$chat_refresh_rate = ' . $chat_refresh_rate . ';', '$chat_refresh_rate = 6;', $output);
  $output = str_replace('$sound_alert_new_message = ' . $sound_alert_new_message . ';', '$sound_alert_new_message = 1;', $output);

  $output = str_replace('$status_indicator_img_type = "' . $status_indicator_img_type . '";', '$status_indicator_img_type = "gif";', $output);
  $output = str_replace('$sound_alert_new_pro_msg = ' . $sound_alert_new_pro_msg . ';', '$sound_alert_new_pro_msg = 1;', $output);
  $output = str_replace('$invitation_position = "' . $invitation_position . '";', '$invitation_position = "right";', $output);

  $f = fopen($activehelper_livehelp['import_dir'] . '/constants.php', 'w');
  fwrite($f, $output);
  fclose($f);
	
	drupal_set_message(t('The server settings have been reseted.'));

	/*
	 * Update the LiveHelp server info file.
	 */
  global $base_path;
	$parsed_url = parse_url( $activehelper_livehelp[ 'base_url' ] );

	$info_host = $parsed_url[ 'scheme' ] . '://' . $parsed_url[ 'host' ];
	$info_domain_set_path = DRUPAL_ROOT . '/' . $activehelper_livehelp[ 'domains_dir' ];
	$info_dir_path = $base_path . str_replace( '\\', '/', $activehelper_livehelp[ 'base_dir' ] );
	$info_conf_path = DRUPAL_ROOT . '/' . $activehelper_livehelp[ 'base_dir' ];
	$info_conf_ssl = $parsed_url[ 'scheme' ] == 'http' ? '0' : '1';

	$info_file = $activehelper_livehelp[ 'import_dir' ] . '/jlhconst.php';
	$output = '<?php

define( "J_HOST", "' . $info_host . '" );
define( "J_DOMAIN_SET_PATH", "' . $info_domain_set_path . '" );
define( "J_DIR_PATH", "' . $info_dir_path . '" );
define( "J_CONF_PATH", "' . $info_conf_path . '" );
define( "J_CONF_SSL", ' . $info_conf_ssl . ' );

?>';

	$f = fopen( $info_file, 'w' );
	fwrite( $f, $output );
	fclose( $f );

	drupal_set_message( t( 'The LiveHelp server information file @file has been updated.', array( '@file' => '/server/import/jlhconst.php' ) ) );

	/*
	 * Save the Drupal database info in the LiveHelp database config file.
	 */
	$config_file = $activehelper_livehelp[ 'import_dir' ] . '/config_database.php';

	$db_info = Database::getConnectionInfo();
	$db_info = isset( $db_info[ 'default' ] ) ? $db_info[ 'default' ] : false;

	// We're going to try to pick up all the info from the settings.php file
	$db_prefix = isset( $db_info[ 'prefix' ][ 'default' ] ) ? $db_info[ 'prefix' ][ 'default' ] : false;
	$db_host = isset( $db_info[ 'host' ] ) ? $db_info[ 'host' ] : false;
	$db_user = isset( $db_info[ 'username' ] ) ? $db_info[ 'username' ] : false;
	$db_pass = isset( $db_info[ 'password' ] ) ? $db_info[ 'password' ] : false;
	$db_name = isset( $db_info[ 'database' ] ) ? $db_info[ 'database' ] : false;
	$db_driver = isset( $db_info[ 'driver' ] ) ? $db_info[ 'driver' ] : false;

	if ( $db_prefix === false ) {
		drupal_set_message( t( 'The LiveHelp database information file @file has not been updated. It was not able to get the database information from the Drupal settings. You will need to update this file manually.', array( '@file' => '/server/import/config_database.php' ) ), 'error' );
	}
	else {
    $output = '<?php

if ( !defined( \'__CONFIG_DATABASE_INC\' ) ) {
	define( \'__CONFIG_DATABASE_INC\', 1 );
	define( \'DS\', DIRECTORY_SEPARATOR );

	include_once(\'constants.php\');
	include_once(\'jlhconst.php\');

	define( "DB_HOST", "' . $db_host . '" );
	define( "DB_USER", "' . $db_user . '" );
	define( "DB_PASS", "' . $db_pass . '" );
	define( "DB_NAME", "' . $db_name . '" );

	$table_prefix = \'' . $db_prefix . '\';
	$table_prefix = $table_prefix . \'livehelp_\';
}

?>';

    $f = fopen( $config_file, 'w' );
    fwrite( $f, $output );
    fclose( $f );

		if ( $db_driver != 'mysql' ) {
			drupal_set_message( t( 'The LiveHelp server does not support any other driver than MySQL. The LiveHelp database information file @file has been updated. but you must notice the LiveHelp server will not work properly.', array( '@file' => '/server/import/config_database.php' ) ), 'error' );
		}
		else {
			drupal_set_message( t( 'The LiveHelp database information file @file has been updated.', array( '@file' => '/server/import/config_database.php' ) ) );
		}
	}

  drupal_goto('admin/activehelper-livehelp/server-settings');
}

