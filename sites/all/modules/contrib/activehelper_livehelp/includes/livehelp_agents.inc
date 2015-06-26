<?php

function activehelper_livehelp__agents() {
  global $activehelper_livehelp;

  $output = '<h2>' . t('Agents') . '</h2>';

  // Set the default order of the table
  if (!isset($_GET['order'])) {
    $_GET['order'] = t('Username');
  }

  if (isset($_GET['agent_delete'])) {
    activehelper_livehelp__agents_delete($_GET['agent_delete']);
  }

  // Prepare table header
  $table_header = array(
    array('data' => t('ID'), 'field' => 'id'),
    array('data' => t('Username'), 'field' => 'username'),
    array('data' => t('Email'), 'field' => 'email'),
    array('data' => t('Department'), 'field' => 'department'),
    array('data' => t('Status'), 'field' => 'status'),
    t('Operations')
  );

  // Set the query params
  /*
    SELECT id, username, status, department, email
    FROM drupal_livehelp_users
    ORDER BY id
   */
  $query = db_select('livehelp_users', 'u')
      ->fields('u', array('id', 'username', 'status', 'department', 'email'))
      // Pagination
      ->extend('TableSort')
      ->extend('PagerDefault')
      ->orderByHeader($table_header)
      ->limit($activehelper_livehelp['listing_limit']);

  // Get the result
  $result = $query->execute();

  // Prepare table rows
  $table_rows = array();

  // Looping for filling the table rows
  while ($data = $result->fetchObject()) {
    // Fill the table rows
    $table_rows[] = array(
      $data->id,
      $data->username,
      $data->email,
      $data->department,
      $data->status ? t('Enabled') : t('Disabled'),
      l(t('Edit'), 'admin/activehelper-livehelp/agents/' . $data->id . '/edit') . ' &nbsp;|&nbsp; ' .
      l(t('Settings'), 'admin/activehelper-livehelp/agents/' . $data->id . '/settings/en') . ' &nbsp;|&nbsp; ' .
      l(t('Client info'), 'admin/activehelper-livehelp/agents/' . $data->id . '/client-info') . ' &nbsp;|&nbsp; ' .      
      l(
          t('Delete'), 'admin/activehelper-livehelp/agents', array(
        'attributes' => array(
          'onclick' => "return window.confirm( '" . t('Are you sure you want to delete the agent with username @agent?', array('@agent' => $data->username)) . "' );"
        ),
        'query' => array(
          'agent_delete' => $data->id
        )
          )
      )
    );
  }

  // Render the table
  $output .= theme_table(array(
        'header' => $table_header,
        'rows' => $table_rows,
        'attributes' => array('width' => '100%'),
        'sticky' => TRUE,
        'caption' => '',
        'colgroups' => array(),
        'empty' => t('No agents found.')
      ))
      . theme('pager');

  return $output;
}

function activehelper_livehelp__agents_add() {
  $output = '<h2>' . t('Add agent') . '</h2>';

  $form = drupal_get_form('activehelper_livehelp__agents_add_edit_form', 'add');
  $output .= render($form);

  return $output;
}

function activehelper_livehelp__agents_edit($agent_id = 0) {
  $agent_id = (int) $agent_id;

  $agent = db_select('livehelp_users', 'u')
      ->fields('u', array('id', 'username'))
      ->condition('u.id', $agent_id, '=')
      ->execute()
      ->fetchAssoc();

  $output = '<h2>' . t('Edit agent @agent', array('@agent' => $agent['username'])) . '</h2>';

  $form = drupal_get_form('activehelper_livehelp__agents_add_edit_form', 'edit', $agent_id);
  $output .= render($form);

  return $output;
}

function activehelper_livehelp__agents_add_edit_form($form, &$form_state) {
  global $activehelper_livehelp;

  $agent_edit = isset($form_state['build_info']['args'][0]) && $form_state['build_info']['args'] [0] == 'edit';

  if (!$agent_edit) {
    $form['#submit'] = array('activehelper_livehelp__agents_add_form_submit');
  }
  else {
    $form['#submit'] = array('activehelper_livehelp__agents_edit_form_submit');

    // Get the domain data from the DB
    $agent_id = (int) $form_state['build_info']['args'][1];
    $agent = db_select('livehelp_users', 'u')
        ->fields('u', array('id', 'username', 'status', 'department', 'email', 'firstname', 'lastname', 'privilege', 'photo', 'answers'))
        ->condition('u.id', $agent_id, '=')
        ->execute()
        ->fetchAssoc();

    $agents_pictures_url = $activehelper_livehelp['agents_pictures_url'];
  }

  // Basic options
  $options_status = array(
    '1' => t('Enabled'),
    '0' => t('Disabled')
  );

  $form['username'] = array(
    '#type' => 'textfield',
    '#title' => t('Username'),
    '#size' => 30,
    '#required' => TRUE,
    '#default_value' => $agent_edit ? $agent['username'] : '',
    '#element_validate' => $agent_edit ? array() : array('activehelper_livehelp__agents_add_edit_form__username')
  );
  $form['password'] = array(
    '#type' => 'password',
    '#title' => t('Password'),
    '#description' => $agent_edit ? t('Only if you want to change the current password.') : '',
    '#size' => 20,
    '#default_value' => ''
  );
  $form['email'] = array(
    '#type' => 'textfield',
    '#title' => t('Email'),
    '#size' => 30,
    '#required' => TRUE,
    '#default_value' => $agent_edit ? $agent['email'] : ''
  );

  $form['information'] = array(
    '#type' => 'fieldset',
    '#title' => t('Information'),
    '#weight' => 10,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE
  );
  $form['information']['firstname'] = array(
    '#type' => 'textfield',
    '#title' => t('First name'),
    '#size' => 30,
    '#required' => TRUE,
    '#default_value' => $agent_edit ? $agent['firstname'] : ''
  );
  $form['information']['lastname'] = array(
    '#type' => 'textfield',
    '#title' => t('Last name'),
    '#size' => 30,
    '#required' => TRUE,
    '#default_value' => $agent_edit ? $agent['lastname'] : ''
  );
  $form['information']['department'] = array(
    '#type' => 'textfield',
    '#title' => t('Department'),
    '#size' => 30,
    '#required' => TRUE,
    '#default_value' => $agent_edit ? $agent['department'] : ''
  );
  $form['information']['status'] = array(
    '#type' => 'radios',
    '#title' => t('Status'),
    '#options' => $options_status,
    '#default_value' => $agent_edit ? $agent['status'] : 1
  );
  $form['information']['privilege'] = array(
    '#type' => 'radios',
    '#title' => t('Domain privilege'),
    '#options' => $options_status,
    '#default_value' => $agent_edit ? $agent['privilege'] : 1
  );
  $form['information']['answers'] = array(
    '#type' => 'radios',
    '#description' => t('Use "Domain" for the global status indicator, if you select "Agent" remember that this Live Chat operator only will be available for the tracking module with the agent ID.'),
    '#title' => t('Status indicator type'),
    '#options' => array(1 => t('Domain'), 2 => t('Agent')),
    '#default_value' => $agent_edit ? $agent['answers'] : 1
  );
  if ($agent_edit && $agent['photo']) {
    $img = $activehelper_livehelp['agents_url'] . '/' . $agent_id . '/' . $agent['photo'] . '?' . rand();
  }
  $form['information']['photo'] = array(
    '#type' => 'file',
    '#title' => t('Picture'),
    '#size' => 80,
    '#description' => $agent_edit && $agent['photo'] ? t('The already uploaded image:') . '<br /><br /><img src="' . check_plain($img) . '" alt="" />' : ''
  );

  $form['domains'] = array(
    '#type' => 'fieldset',
    '#title' => t('Active domains'),
    '#weight' => 20,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE
  );

  // Let's pick the current domain status for this agent
  $agent_domains = array();
  if ($agent_edit) {
    $query = db_select('livehelp_domain_user', 'du')
        ->fields('du', array('id_domain', 'status'))
        ->condition('du.id_user', $agent_id, '=');

    $result = $query->execute();
    while ($data = $result->fetchObject()) {
      $agent_domains[$data->id_domain] = $data->status;
    }
  }

  $query = db_select('livehelp_domains', 'd')
      ->fields('d', array('id_domain', 'name', 'status'));
  $result = $query->execute();

  while ($data = $result->fetchObject()) {
    $form['domains']['domain__' . $data->id_domain] = array(
      '#type' => 'radios',
      '#title' => check_plain($data->name),
      '#options' => $options_status,
      '#default_value' => $agent_edit ? (!empty($agent_domains[$data->id_domain]) ? 1 : 0 ) : 0
    );
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#weight' => 30,
    '#value' => t('Save'),
  );

  return $form;
}

function activehelper_livehelp__agents_add_edit_form__username($element, &$form_state) {
  $agent = db_select('livehelp_users', 'u')
      ->fields('u', array('id'))
      ->condition('u.username', $element['#value'], '=')
      ->execute()
      ->fetchAssoc();

  if (!empty($agent)) {
    form_error($element, t('The username @username is already in use.', array('@username' => $element['#value'])));
  }
}

function activehelper_livehelp__agents_add_form_submit($form, &$form_state) {
  global $activehelper_livehelp;

  $agents_pictures_dir = $activehelper_livehelp['agents_pictures_dir'];

  $agent_id = db_insert('livehelp_users')
      ->fields(array(
        'username' => $form_state['values']['username'],
        'email' => $form_state['values']['email'],
        'firstname' => $form_state['values']['firstname'],
        'lastname' => $form_state['values']['lastname'],
        'department' => $form_state['values']['department'],
        'status' => $form_state['values']['status'],
        'privilege' => $form_state['values']['privilege'],
        'answers' => $form_state['values']['answers'],
        'datetime' => '1900-01-01 00:00:00',
        'refresh' => '1900-01-01 00:00:00',
      ))
      ->execute();

  $form_state['redirect'] = 'admin/activehelper-livehelp/agents/' . $agent_id . '/client-info';

  foreach ($form_state['values'] as $k => $v) {
    if (!empty($v) && strstr($k, 'domain__')) {
      $domain_id = (int) str_replace('domain__', '', $k);

      $relation_id = db_insert('livehelp_domain_user')
          ->fields(array(
            'id_domain' => $domain_id,
            'id_user' => $agent_id,
            'status' => 1
          ))
          ->execute();

      db_insert('livehelp_sa_domain_user_role')
          ->fields(array(
            'id_domain_user' => $relation_id,
            'id_role' => 1
          ))
          ->execute();
    }
  }
  
  // Let's import the files lib
  module_load_include('inc', 'activehelper_livehelp', 'includes/livehelp_lib_files');
  
	// Duplicate agent folder with id 0 for this new agent.
  activehelper_livehelp__lib_files_duplicate(
      $activehelper_livehelp['agents_dir'] . '/0', $activehelper_livehelp['agents_dir'] . '/' . $agent_id
  );

  // Let's import the images lib
  module_load_include('inc', 'activehelper_livehelp', 'includes/livehelp_lib_images');
  $form_files = activehelper_livehelp__helper_multiple_files($_FILES);
  if (isset($form_files['files'])) {
    $form_files = $form_files['files'];
  }

  if (isset($form_files['photo']['error']) && $form_files['photo']['error'] == 0) {
    $agent_picture = activehelper_livehelp__lib_images_upload(DRUPAL_ROOT . '/' . $activehelper_livehelp['agents_dir'] . '/' . $agent_id, 'a' . $agent_id, $form_files['photo']);
    if (!is_array($agent_picture)) {
      // If $file is a string, it's an error.
      drupal_set_message(check_plain($agent_picture), 'error');
    }
    else {
      db_query("
				UPDATE {livehelp_users}
				SET photo = '" . $agent_picture['basename'] . "'
				WHERE id = '{$agent_id}'
			");
    }
  }

  if (!empty($form_state['values']['password'])) {
    db_query("
			UPDATE {livehelp_users}
			SET password = '" . md5($form_state['values']['password']) . "'
			WHERE id = '{$agent_id}'
		");
  }

  if ($form_state['values']['status']) {
    drupal_set_message(t('The agent @agent has been registered.', array('@agent' => $form_state['values']['username'])));
  }
  else {
    drupal_set_message(t('The agent @agent has been registered as disabled.', array('@agent' => $form_state['values']['username'])), 'warning');
  }

  return $form;
}

function activehelper_livehelp__agents_edit_form_submit($form, &$form_state) {
  global $activehelper_livehelp;

  $form_state['redirect'] = 'admin/activehelper-livehelp/agents';

  $agent_id = (int) $form_state['build_info']['args'][1];
  $agents_pictures_dir = $activehelper_livehelp['agents_pictures_dir'];

  $agent = db_select('livehelp_users', 'u')
      ->fields('u', array('id', 'photo'))
      ->condition('u.id', $agent_id, '=')
      ->execute()
      ->fetchAssoc();

  db_update('livehelp_users')
      ->fields(array(
        'username' => $form_state['values']['username'],
        'email' => $form_state['values']['email'],
        'firstname' => $form_state['values']['firstname'],
        'lastname' => $form_state['values']['lastname'],
        'department' => $form_state['values']['department'],
        'status' => $form_state['values']['status'],
        'privilege' => $form_state['values']['privilege'],
        'answers' => $form_state['values']['answers']
      ))
      ->condition('id', $agent_id, '=')
      ->execute();

  db_query("
		DELETE FROM {livehelp_sa_domain_user_role}
		WHERE id_domain_user IN (
			SELECT id_domain_user
			FROM {livehelp_domain_user}
			WHERE id_user = {$agent_id}
		)
	");
  db_query("
		DELETE FROM {livehelp_domain_user}
		WHERE id_user = {$agent_id}
	");

  foreach ($form_state['values'] as $k => $v) {
    if (!empty($v) && strstr($k, 'domain__')) {
      $domain_id = (int) str_replace('domain__', '', $k);

      $relation_id = db_insert('livehelp_domain_user')
          ->fields(array(
            'id_domain' => $domain_id,
            'id_user' => $agent_id,
            'status' => 1
          ))
          ->execute();

      db_insert('livehelp_sa_domain_user_role')
          ->fields(array(
            'id_domain_user' => $relation_id,
            'id_role' => 1
          ))
          ->execute();
    }
  }

  // Let's import the images lib
  module_load_include('inc', 'activehelper_livehelp', 'includes/livehelp_lib_images');
  $form_files = activehelper_livehelp__helper_multiple_files($_FILES);
  if (isset($form_files['files'])) {
    $form_files = $form_files['files'];
  }

  if (isset($form_files['photo']['error']) && $form_files['photo']['error'] == 0) {
    if (!empty($agent['photo'])) {
      module_load_include('inc', 'activehelper_livehelp', 'includes/livehelp_lib_files');
      activehelper_livehelp__lib_files_delete($activehelper_livehelp['agents_dir'] . '/' . $agent_id . '/' . $agent['photo']);
    }

    $agent_picture = activehelper_livehelp__lib_images_upload(DRUPAL_ROOT . '/' . $activehelper_livehelp['agents_dir'] . '/' . $agent_id, 'a' . $agent_id, $form_files['photo']);
    if (!is_array($agent_picture)) {
      // If $file is a string, it's an error.
      drupal_set_message(check_plain($agent_picture), 'error');
    }
    else {
      db_query("
				UPDATE {livehelp_users}
				SET photo = '" . $agent_picture['basename'] . "'
				WHERE id = '{$agent_id}'
			");
    }
  }

  if (!empty($form_state['values']['password'])) {
    db_query("
			UPDATE {livehelp_users}
			SET password = '" . md5($form_state['values']['password']) . "'
			WHERE id = '{$agent_id}'
		");
  }

  drupal_set_message(t('The agent has been updated.'));

  return $form;
}

function activehelper_livehelp__agents_delete($agent_id = 0) {
  global $activehelper_livehelp;

  $agent_id = (int) $agent_id;
  $agents_pictures_dir = $activehelper_livehelp['agents_pictures_dir'];

  $agent = db_select('livehelp_users', 'u')
      ->fields('u', array('id', 'username', 'status', 'department', 'email', 'firstname', 'lastname', 'privilege', 'photo'))
      ->condition('u.id', $agent_id, '=')
      ->execute()
      ->fetchAssoc();

  if (empty($agent)) {
    drupal_set_message(t('The agent with ID @id does not exist.', array('@id' => $agent_id)), 'error');
  }
  else {
    db_query("
			DELETE FROM {livehelp_sa_domain_user_role}
			WHERE id_domain_user IN (
				SELECT id_domain_user
				FROM {livehelp_domain_user}
				WHERE id_user = {$agent_id}
			)
		");
    db_query("
			DELETE FROM {livehelp_domain_user}
			WHERE id_user = {$agent_id}
		");
    db_query("
			DELETE FROM {livehelp_users}
			WHERE id = {$agent_id}
		");

    module_load_include('inc', 'activehelper_livehelp', 'includes/livehelp_lib_files');
    if (!empty($agent['photo'])) {
      activehelper_livehelp__lib_files_delete($agents_pictures_dir . '/' . $agent['photo']);
    }

    drupal_set_message(t('The agent @agent has been removed.', array('@agent' => $agent['username'])), 'status');
  }
}

function activehelper_livehelp__agents_client_info($agent_id = 0) {
  $agent_id = (int) $agent_id;

  $agent = db_select('livehelp_users', 'u')
      ->fields('u', array('id', 'username'))
      ->condition('u.id', $agent_id, '=')
      ->execute()
      ->fetchAssoc();

  $output = '<h2>' . t('Client info for @agent', array('@agent' => $agent['username'])) . '</h2>';

  $form = drupal_get_form('activehelper_livehelp__agents_client_info_form', $agent['username']);
  $output .= render($form);

  return $output;
}

function activehelper_livehelp__agents_client_info_form($form, &$form_state) {
  global $activehelper_livehelp;

  $agent_name = $form_state['build_info']['args'][0];

  $parsed_url = parse_url($activehelper_livehelp['server_url']);

  $client_info_server = $parsed_url['scheme'] . '://' . $parsed_url['host'];
  $client_info_server_path = str_replace($client_info_server, '', $activehelper_livehelp['server_url']) . '/';
  $client_info_username = $agent_name;
  $client_info_ssl = $parsed_url['scheme'] == 'http' ? 'OFF' : 'ON';

  $form['client'] = array(
    '#type' => 'fieldset',
    '#title' => t('Client Info'),
    '#weight' => 1,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE
  );
  
  $form['client']['server'] = array(
    '#type' => 'textfield',
    '#title' => t('Server'),
    '#size' => 30,
    '#default_value' => $client_info_server,
    '#attributes' => array(
      'onclick' => 'this.select();',
      'readonly' => 'readonly'
    )
  );
  $form ['client']['path'] = array(
    '#type' => 'textfield',
    '#title' => t('Server path'),
    '#size' => 50,
    '#default_value' => $client_info_server_path,
    '#attributes' => array(
      'onclick' => 'this.select();',
      'readonly' => 'readonly'
    )
  );
  $form['client']['account'] = array(
    '#type' => 'textfield',
    '#title' => t('Account'),
    '#size' => 30,
    '#default_value' => 'default',
    '#attributes' => array(
      'onclick' => 'this.select();',
      'readonly' => 'readonly'
    )
  );
  $form['client']['login']= array(
    '#type' => 'textfield',
    '#title' => t('Login'),
    '#size' => 30,
    '#default_value' => $client_info_username,
    '#attributes' => array(
      'onclick' => 'this.select();',
      'readonly' => 'readonly'
    )
  );
  $form['client']['ssl'] = array(
    '#type' => 'textfield',
    '#title' => t('SSL'),
    '#size' => 30,
    '#default_value' => $client_info_ssl,
    '#attributes' => array(
      'readonly' => 'readonly',
      'onclick' => 'this.select();',
    ),
  );
  
  $form['Downloads'] = array(
    '#type' => 'fieldset',
    '#title' => t('Available Downloads'),
    '#weight' => 14,
    '#collapsible' => TRUE,
    '#collapsed' => TRUE
  );
 
 $form['Downloads'] ['Links']  = array(
  array('#type' => 'link',
  '#title' => t('Support Panel Desktop for Windows'),
  '#href' => 'http://www.activehelper.com/downloads/client/windows/installer-supportpanel.exe', 
  '#suffix' => '<br>'),
 
   array('#type' => 'link',
  '#title' => t('Support Panel Desktop for MAC'),
  '#href' => 'http://www.activehelper.com/downloads/client/mac/installer-supportpanel.dmg', 
  '#suffix' => '<br>'),
  
  array('#type' => 'link',
  '#title' => t('Support Panel Mobile for IOS'),
  '#href' => 'https://itunes.apple.com/us/app/live-help/id515929709?mt=8', 
  '#suffix' => '<br>'),
  
  array('#type' => 'link',
  '#title' => t('Support Panel mobile for Android'),
  '#href' => 'https://play.google.com/store/apps/details?id=air.com.activehelper.supportpanel',
  '#suffix' => '<br>'),
  
   array('#type' => 'link',
  '#title' => t('Support Panel Desktop User Guide'),
  '#href' => 'href="http://www.slideshare.net/activehelper/support-panel-console-3-user-guide',
  '#suffix' => '<br>'),
  
   array('#type' => 'link',
  '#title' => t('Support Panel Mobile User Guide'),
  '#href' => 'http://www.slideshare.net/activehelper/support-panel-mobile-user-guide-for-iphone-and-ipad-english')    
         
);

  $form['submit'] = array(
    '#type' => 'submit',
    '#weight' => 15,
    '#value' => t('Back'),
  );

  return $form;
}

function activehelper_livehelp__agents_client_info_form_submit($form, &$form_state) {
  $form_state['redirect'] = 'admin/activehelper-livehelp/agents';

  return $form;
}

function activehelper_livehelp__agents_settings($agent_id = 0, $agent_language = 'en') {
  $agent_id = (int) $agent_id;

  $agent = db_select('livehelp_users', 'u')
      ->fields('u', array('id', 'username'))
      ->condition('u.id', $agent_id, '=')
      ->execute()
      ->fetchAssoc();

  $output = '<h2>' . t('Settings for @agent', array('@agent' => $agent['username'])) . '</h2>';

  $form = drupal_get_form('activehelper_livehelp__agents_settings_form', $agent_id, $agent_language);
  $output .= render($form);

  $output .= '
		<script type="text/javascript">
			function livehelp_images_language( select ) {
				window.location = "' . url('admin/activehelper-livehelp/agents/' . $agent_id) . '/settings/" + select.options[ select.selectedIndex ].value + "";
			}
		</script>
	';

  return $output;
}

function activehelper_livehelp__agents_settings_form($form, &$form_state) {
  global $activehelper_livehelp;

    $agent_id = $form_state['build_info']['args'][0];
    $agent_language = $form_state['build_info']['args'][1];
    $agent_edit = isset($form_state['build_info']['args'][0]);
  
    $agent_url = sprintf($activehelper_livehelp['agents_pictures_url2'], $agent_id, $agent_language);
	$agent_dir = sprintf($activehelper_livehelp['agents_pictures_dir2'], $agent_id, $agent_language);

	$agent_imgs_paths = array_filter(glob($agent_dir . '/*'), 'is_file');
	$agent_imgs = array();
	foreach ($agent_imgs_paths as $path) {
		$agent_imgs[basename($path, '.gif')] = basename($path, '.gif');
	}

    
    $agent = db_select('livehelp_users', 'u')
        ->fields('u', array('id', 'schedule', 'initial_time', 'final_time'))
        ->condition('u.id', $agent_id, '=')
        ->execute()
        ->fetchAssoc();

  // To easily get the text of a language by code. We'll need this later.
  $languages_texts = array(
    'en' => t('English'),
    'sp' => t('Spanish'),
    'de' => t('Deutsch'),
    'pt' => t('Portuguese'),
    'it' => t('Italian'),
    'fr' => t('French'),
    'cz' => t('Czech'),
    'se' => t('Swedish'),
    'no' => t('Norwegian'),
    'tr' => t('Turkey'),
    'gr' => t('Greek'),
    'he' => t('Hebrew'),
    'fa' => t('Farsi'),
    'sr' => t('Serbian'),
    'ru' => t('Rusian'),
    'hu' => t('Hungarian'),
    'zh' => t('Traditional Chinese'),
    'ar' => t('Arab'),
    'nl' => t('Dutch'),
    'fi' => t('Finnish'),
    'dk' => t('Danish'),
    'pl' => t('Polish'),
    'cn' => t('Simplified Chinese'),
    'bg' => t('Bulgarian'),
    'sk' => t('Slovak'),
    'cr' => t('Croatian'),
    'id' => t('Indonesian'),
    'lt' => t('Lithuanian'),
    'ro' => t('Romanian'),
    'sl' => t('Slovenian'),
    'et' => t('Estonian'),
    'lv' => t('Latvian'),
    'ge' => t('Georgian')
  );
  
  // Basic options
  $options_status = array(
    '1' => t('Enabled'),
    '0' => t('Disabled')
  );
  
  $form['language'] = array(
    '#type' => 'fieldset',
    '#title' => t('Language'),
    '#weight' => 5,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE
  );
  $form['language']['images_language'] = array(
    '#attributes' => array(
      'onchange' => 'livehelp_images_language( this )'
    ),
    '#type' => 'select',
    '#title' => t('Lenguage of the images'),
    '#options' => $languages_texts,
    '#default_value' => $agent_language,
    '#description' => t('Select the language you would like to edit the images below for.'),
  );
  

  $form['images'] = array(
    '#type' => 'fieldset',
    '#title' => t('Images'),
    '#weight' => 10,
    '#collapsible' => TRUE,
    '#collapsed' => TRUE
  );
  $form['images']['offline'] = array(
    '#type' => 'file',
    '#title' => t('Offline image (gif)'),
    '#size' => 80,
    '#description' => t('The already uploaded image:') . '<br /><br /><img src="' . $agent_url . '/offline.gif' . '?' . rand() . '" alt="" />'
  );
  $form['images']['online'] = array(
    '#type' => 'file',
    '#title' => t('Online image (gif)'),
    '#size' => 80,
    '#description' => t('The already uploaded image:') . '<br /><br /><img src="' . $agent_url . '/online.gif' . '?' . rand() . '" alt="" />'
  );
  $form['images']['away'] = array(
    '#type' => 'file',
    '#title' => t('Away image (gif)'),
    '#size' => 80,
    '#description' => t('The already uploaded image:') . '<br /><br /><img src="' . $agent_url . '/away.gif' . '?' . rand() . '" alt="" />'
  );
  $form['images']['brb'] = array(
    '#type' => 'file',
    '#title' => t('BRB image (gif)'),
    '#size' => 80,
    '#description' => t('The already uploaded image:') . '<br /><br /><img src="' . $agent_url . '/brb.gif' . '?' . rand() . '" alt="" />'
  );
  
    $form['Schedule'] = array(
    '#type' => 'fieldset',
    '#title' => t('Schedule'),
    '#weight' => 14,
    '#collapsible' => TRUE,
    '#collapsed' => TRUE
  );
  
  $form['Schedule']['schedule'] = array(
    '#type' => 'radios',
    '#title' => t('Schedule'),
    '#options' => $options_status,
    '#default_value' => $agent_edit ? $agent['schedule'] : 0
  );
  
  $form['Schedule']['initial_time'] = array(
    '#type' => 'textfield',
    '#title' => t('Initial Time'),
    '#size' => 30,
    '#required' => TRUE,
    '#default_value' => $agent_edit ? $agent['initial_time'] : '',
    '#element_validate' => $agent_edit ? array() : array('activehelper_livehelp__agents_add_edit_form__initial_time')
     );
  
  $form['Schedule']['final_time'] = array(
    '#type' => 'textfield',
    '#title' => t('Final Time'),
    '#size' => 30,
    '#required' => TRUE,
    '#default_value' => $agent_edit ? $agent['final_time'] : '',
    '#element_validate' => $agent_edit ? array() : array('activehelper_livehelp__agents_add_edit_form__final_time')
  );
    
  $form['submit'] = array(
    '#type' => 'submit',
    '#weight' => 15,
    '#value' => t('Save'),
  );

  return $form;
}

function activehelper_livehelp__agents_settings_form_submit($form, &$form_state) {
  global $activehelper_livehelp;

  $agent_id = $form_state['build_info']['args'][0];
  $agent_language = $form_state['build_info']['args'][1];
  
  $agent_url = sprintf($activehelper_livehelp['agents_pictures_url2'], $agent_id, $agent_language);
	$agent_dir = sprintf($activehelper_livehelp['agents_pictures_dir2'], $agent_id, $agent_language);

  // To easily get the text of a language by code. We'll need this later.
  $languages_texts = array(
    'en' => t('English'),
    'sp' => t('Spanish'),
    'de' => t('Deutsch'),
    'pt' => t('Portuguese'),
    'it' => t('Italian'),
    'fr' => t('French'),
    'cz' => t('Czech'),
    'se' => t('Swedish'),
    'no' => t('Norwegian'),
    'tr' => t('Turkey'),
    'gr' => t('Greek'),
    'he' => t('Hebrew'),
    'fa' => t('Farsi'),
    'sr' => t('Serbian'),
    'ru' => t('Rusian'),
    'hu' => t('Hungarian'),
    'zh' => t('Traditional Chinese'),
    'ar' => t('Arab'),
    'nl' => t('Dutch'),
    'fi' => t('Finnish'),
    'dk' => t('Danish'),
    'pl' => t('Polish'),
    'cn' => t('Simplified Chinese'),
    'bg' => t('Bulgarian'),
    'sk' => t('Slovak'),
    'cr' => t('Croatian'),
    'id' => t('Indonesian'),
    'lt' => t('Lithuanian'),
    'ro' => t('Romanian'),
    'sl' => t('Slovenian'),
    'et' => t('Estonian'),
    'lv' => t('Latvian'),
    'ge' => t('Georgian')
  );

  // Let's import the images lib
  module_load_include('inc', 'activehelper_livehelp', 'includes/livehelp_lib_images');

  // Get the files variable in a friendly format.
  $form_files = activehelper_livehelp__helper_multiple_files($_FILES);
  if (isset($form_files['files'])) {
    $form_files = $form_files['files'];
  }

  // Upload the offline first
  if (isset($form_files['offline']['error']) && $form_files['offline']['error'] == 0) {
    $file = activehelper_livehelp__lib_images_upload(DRUPAL_ROOT . '/' . $agent_dir, 'offline', $form_files['offline'], '.gif');
    if (!is_array($file)) {
      // If $file is a string, it's an error.
      drupal_set_message(check_plain($file), 'error');
    }
  }

  // Upload the online first
  if (isset($form_files['online']['error']) && $form_files['online']['error'] == 0) {
    $file = activehelper_livehelp__lib_images_upload(DRUPAL_ROOT . '/' . $agent_dir, 'online', $form_files['online'], '.gif');
    if (!is_array($file)) {
      // If $file is a string, it's an error.
      drupal_set_message(check_plain($file), 'error');
    }
  }

  // Upload the away first
  if (isset($form_files['away']['error']) && $form_files['away']['error'] == 0) {
    $file = activehelper_livehelp__lib_images_upload(DRUPAL_ROOT . '/' . $agent_dir, 'away', $form_files['away'], '.gif');
    if (!is_array($file)) {
      // If $file is a string, it's an error.
      drupal_set_message(check_plain($file), 'error');
    }
  }

  // Upload the brb first
  if (isset($form_files['brb']['error']) && $form_files['brb']['error'] == 0) {
    $file = activehelper_livehelp__lib_images_upload(DRUPAL_ROOT . '/' . $agent_dir, 'brb', $form_files['brb'], '.gif');
    if (!is_array($file)) {
      // If $file is a string, it's an error.
      drupal_set_message(check_plain($file), 'error');
    }
  }

// Update agent schedule
  db_update('livehelp_users')
      ->fields(array(
        'schedule' => $form_state['values']['schedule'],
        'initial_time' => $form_state['values']['initial_time'],
        'final_time' => $form_state['values']['final_time']
        ))
      ->condition('id', $agent_id, '=')
      ->execute();

  // Save the images
  drupal_set_message(t('The agent settings have been updated.'));

  return $form;
}
