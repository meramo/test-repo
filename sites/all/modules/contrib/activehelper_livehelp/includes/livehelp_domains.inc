<?php

function activehelper_livehelp__domains() {
  global $base_url, $base_path;
  global $activehelper_livehelp;

  $output = '<h2>' . t('Domains') . '</h2>';

  // Set the default order of the table
  if (!isset($_GET['order'])) {
    $_GET['order'] = t('Domain name');
  }

  if (isset($_GET['domain_delete'])) {
    activehelper_livehelp__domains_delete($_GET['domain_delete']);
  }

  // Prepare table header
  $table_header = array(
    array('data' => t('ID'), 'field' => 'id_domain'),
    array('data' => t('Domain name'), 'field' => 'name'),
    t('Tracking block'),
    t('Tracking script'),
    array('data' => t('Status'), 'field' => 'status'),
    t('Operations')
  );

  // Set the query params
  /*
    SELECT id_domain, name, status
    FROM drupal_livehelp_domains
    ORDER BY id_domain
   */
  $query = db_select('livehelp_domains', 'd')
      ->fields('d', array('id_domain', 'name', 'status'))
      // Pagination
      ->extend('TableSort')
      ->extend('PagerDefault')
      ->orderByHeader($table_header)
      ->limit($activehelper_livehelp['listing_limit']);

  // Get the result
  $result = $query->execute();


  $base_url = $base_url;
  $domain_name = parse_url($base_url);
  $domain_name = $domain_name['host'];

  // Prepare table rows
  $table_rows = array();

  // Looping for filling the table rows
  while ($data = $result->fetchObject()) {
    // Fill the table rows
    $table_rows[] = array(
      $data->id_domain,
      $data->name,
      ( $data->name == $domain_name ? l(t('Configure'), 'admin/structure/block/manage/activehelper_livehelp/block/configure') . ' &nbsp;|&nbsp; ' : '' ) .
      l(t('Download'), 'admin/activehelper-livehelp/domains/' . $data->id_domain . '/download-block'),
      l(t('Generate'), 'admin/activehelper-livehelp/domains/' . $data->id_domain . '/tracking-script'),
      $data->status ? t('Enabled') : t('Disabled'),
      l(t('Edit'), 'admin/activehelper-livehelp/domains/' . $data->id_domain . '/edit') . ' &nbsp;|&nbsp; ' .
      l(t('Settings'), 'admin/activehelper-livehelp/domains/' . $data->id_domain . '/en/settings') . ' &nbsp;|&nbsp; ' .      
      l(
          t('Delete'), 'admin/activehelper-livehelp/domains', array(
        'attributes' => array(
          'onclick' => "return window.confirm( '" . t('Are you sure you want to delete the domain @domain?', array('@domain' => $data->name)) . "' );"
        ),
        'query' => array(
          'domain_delete' => $data->id_domain
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
        'empty' => t('No domains found.')
      ))
      . theme('pager');

  return $output;
}

function activehelper_livehelp__domains_add() {
  $output = '<h2>' . t('Add domain') . '</h2>';

  $form = drupal_get_form('activehelper_livehelp__domains_add_edit_form', 'add');
  $output .= render($form);

  return $output;
}

function activehelper_livehelp__domains_edit($domain_id = 0) {
  $domain_id = (int) $domain_id;

  $domain = db_select('livehelp_domains', 'd')
      ->fields('d', array('id_domain', 'name', 'status'))
      ->condition('d.id_domain', $domain_id, '=')
      ->execute()
      ->fetchAssoc();

  $output = '<h2>' . t('Edit domain @domain', array('@domain' => $domain['name'])) . '</h2>';

  $form = drupal_get_form('activehelper_livehelp__domains_add_edit_form', 'edit', $domain_id);
  $output .= render($form);

  return $output;
}

function activehelper_livehelp__domains_add_edit_form($form, &$form_state) {
  $domain_edit = isset($form_state['build_info']['args'][0]) && $form_state['build_info']['args'] [0] == 'edit';

  if (!$domain_edit) {
    $form['#submit'] = array('activehelper_livehelp__domains_add_form_submit');
  }
  else {
    $form['#submit'] = array('activehelper_livehelp__domains_edit_form_submit');

    // Get the domain data from the DB
    $domain_id = (int) $form_state['build_info']['args'][1];
    $domain = db_select('livehelp_domains', 'd')
        ->fields('d', array('id_domain', 'name', 'status'))
        ->condition('d.id_domain', $domain_id, '=')
        ->execute()
        ->fetchAssoc();
  }

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#size' => 30,
    '#description' => t('Example: www.example.com'),
    '#required' => TRUE,
    '#default_value' => $domain_edit ? $domain['name'] : '',
    '#element_validate' => $domain_edit ? array() : array('activehelper_livehelp__domains_add_edit_form__name')
  );
  $form['status'] = array(
    '#type' => 'radios',
    '#title' => t('Status'),
    '#options' => array('1' => t('Enabled'), '0' => t('Disabled')),
    '#default_value' => $domain_edit ? $domain['status'] : 1
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

function activehelper_livehelp__domains_add_edit_form__name($element, &$form_state) {
  $domain = db_select('livehelp_domains', 'd')
      ->fields('d', array('id_domain'))
      ->condition('d.name', $element['#value'], '=')
      ->execute()
      ->fetchAssoc();

  if (!empty($domain)) {
    form_error($element, t('The domain @domain is already registered.', array('@domain' => $element['#value'])));
  }
}

function activehelper_livehelp__domains_add_form_submit($form, &$form_state) {
  global $activehelper_livehelp;

  $form_state['redirect'] = 'admin/activehelper-livehelp/domains';

  $domain_id = db_insert('livehelp_domains')
      ->fields(array(
        'name' => $form_state['values']['name'],
        'status' => $form_state['values']['status'],
        'configuration' => ''
      ))
      ->execute();

  // Create default settings.
  activehelper_livehelp__domains_settings_add_defaults($domain_id);

  // Let's import the files lib
  module_load_include('inc', 'activehelper_livehelp', 'includes/livehelp_lib_files');
  // Duplicate the server files needed for this domain
  activehelper_livehelp__lib_files_duplicate(
      $activehelper_livehelp['domains_dir'] . '/0', $activehelper_livehelp['domains_dir'] . '/' . $domain_id
  );

  // Insert the default domain language
  db_query("
			INSERT INTO {livehelp_languages_domain}
				( Id_domain, code, name, welcome_message )
			VALUES
				( '{$domain_id}', 'en', 'English', 'Welcome to our LiveHelp, one moment please.' )
	");
  // Insert the default account relationship
  db_query("
			INSERT INTO {livehelp_accounts_domain}
				( id_account, id_domain, status )
			VALUES
				( '1', '{$domain_id}', '1' )
	");

  if ($form_state['values']['status']) {
    drupal_set_message(sprintf(
            t('The domain %s has been registered.'), $form_state['values']['name']
        ));
  }
  else {
    drupal_set_message(
        sprintf(
            t('The domain %s has been registered as disabled.'), $form_state['values']['name']
        ), 'warning'
    );
  }

  return $form;
}

function activehelper_livehelp__domains_edit_form_submit($form, &$form_state) {
  $form_state['redirect'] = 'admin/activehelper-livehelp/domains';

  $domain_id = (int) $form_state['build_info']['args'][1];

  db_update('livehelp_domains')
      ->fields(array(
        'name' => $form_state['values']['name'],
        'status' => $form_state['values']['status']
      ))
      ->condition('id_domain', $domain_id, '=')
      ->execute();

  drupal_set_message(t('The domain @domain has been updated.', array('@domain' => $form_state['values']['name'])));

  return $form;
}

function activehelper_livehelp__domains_delete($domain_id = 0) {
  global $activehelper_livehelp;

  $domain_id = (int) $domain_id;
  $domain_dir = $activehelper_livehelp['domains_dir'] . '/' . $domain_id;

  $domain = db_select('livehelp_domains', 'd')
      ->fields('d', array('id_domain', 'name', 'status'))
      ->condition('d.id_domain', $domain_id, '=')
      ->execute()
      ->fetchAssoc();

  if (empty($domain)) {
    drupal_set_message(t('The domain with ID @id does not exist.', array('@id' => $domain_id)), 'error');
  }
  else {
    db_query("
			DELETE FROM {livehelp_accounts_domain}
			WHERE id_domain = '{$domain_id}'
		");
    db_query("
			DELETE FROM {livehelp_languages_domain}
			WHERE Id_domain = '{$domain_id}'
		");
    db_query("
			DELETE FROM {livehelp_settings}
			WHERE id_domain = '{$domain_id}'
		");
    db_query("
			DELETE FROM {livehelp_domains}
			WHERE id_domain = '{$domain_id}'
		");

    module_load_include('inc', 'activehelper_livehelp', 'includes/livehelp_lib_files');
    activehelper_livehelp__lib_files_delete($domain_dir);

    drupal_set_message(t('The domain @domain has been removed.', array('@domain' => $domain['name'])));
  }
}

function activehelper_livehelp__domains_tracking_script($domain_id = 0) {
  global $activehelper_livehelp;

  $domain_id = (int) $domain_id;

  $domain = db_select('livehelp_domains', 'd')
      ->fields('d', array('id_domain', 'name', 'status'))
      ->condition('d.id_domain', $domain_id, '=')
      ->execute()
      ->fetchAssoc();

  $output = '<h2>' . t('Tracking script for @domain', array('@domain' => $domain['name'])) . '</h2>';

  $form = drupal_get_form('activehelper_livehelp__domains_tracking_script_form', $domain_id);
  $output .= render($form);

  $output .= '
		<script type="text/javascript">
			var scriptLanguage = "en";
      var scriptAgent = 0;
			var scriptTracking = 1;
			var scriptStatus = 1;
			function activehelper_livehelp_generate_script()
			{
				var html ="<p class=\"pin\"><span style=\"font-size: 10pt;\"><div style=\"position: fixed; bottom: 0px; right:0px; z-index:999999999999; display:block;\">\n";                 
                html +="<" + "script type=\"text/javascript\" src=\"' . $activehelper_livehelp['import_url'] . '/javascript.php\">";
				html += "</" + "script>\n";
				html += "<" + "script type=\"text/javascript\">\n";
				html += "	_vlDomain = ' . $domain_id . ';\n";
				html += "	_vlAgent = " + scriptAgent + ";\n";
				html += "	_vlService = 1;\n";
				html += "	_vlLanguage = \"" + scriptLanguage + "\";\n";
				html += "	_vlTracking = " + scriptTracking + ";\n";
				html += "	_vlStatus_indicator = " + scriptStatus + ";\n";
				html += "	startLivehelp();\n";
				html += "</" + "script>";

				jQuery( "#edit-tracking-script" ).val( html );
			}
			jQuery( document ).ready( function( $ ) {

        $("#edit-script-status-indicator-type").change(function() {
          if ($("#edit-script-status-indicator-type").val() == 1) {
            scriptAgent = $("#edit-script-agent").val();
            
            activehelper_livehelp_generate_script();
          }
          else {
            scriptAgent = 0;
            
            activehelper_livehelp_generate_script();
          }
        });

        $("#edit-script-agent").change(function(){ 
          $("#edit-script-status-indicator-type").val(1);
          scriptAgent = $("#edit-script-agent").val();

          activehelper_livehelp_generate_script();
        });
        
				$( "#edit-status-indicator-1" ).click( function(){ scriptStatus = 1; activehelper_livehelp_generate_script(); } );
				$( "#edit-status-indicator-0" ).click( function(){ scriptStatus = 0; activehelper_livehelp_generate_script(); } );

				$( "#edit-tracking-1" ).click( function() { scriptTracking = 1; activehelper_livehelp_generate_script(); } );
				$( "#edit-tracking-0" ).click( function() { scriptTracking = 0; activehelper_livehelp_generate_script(); } );

				$( "#edit-language" ).change( function() { scriptLanguage = $(this).val(); activehelper_livehelp_generate_script(); } );
			} );
		</script>
	';

  return $output;
}

function activehelper_livehelp__domains_tracking_script_form($form, &$form_state) {
  global $activehelper_livehelp;

  $domain_id = (int) $form_state['build_info']['args'][0];

  $query = db_select('livehelp_users', 'u')
      ->fields('u', array('id', 'username'));
  $result = $query->execute();

  $agents_select = array();
  while ($data = $result->fetchObject()) {
    $agents_select[$data->id] = $data->username;
  }

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

  // Load the domain languages
  $domain_languages = array();
  $domain_languages_result = db_query("
		SELECT ld.code
		FROM {livehelp_languages_domain} AS ld
		WHERE ld.id_domain = '{$domain_id}'
	");
  while ($r = $domain_languages_result->fetchAssoc()) {
    $domain_languages[$r['code']] = $languages_texts[$r['code']];
  }

  $script = '<"p class="pin"><span style="font-size: 10pt;"><div style="position: fixed; bottom: 0px; Right:0px; z-index:999999999999; display:block;">
  <script type="text/javascript" src="' . $activehelper_livehelp['import_url'] . '/javascript.php"></script>
<script type="text/javascript">
	_vlDomain = ' . $domain_id . ';
	_vlAgent = 0;
	_vlService = 1;
	_vlLanguage = "en";
	_vlTracking = 1;
	_vlStatus_indicator = 1;
	startLivehelp();
</script>';

  // Basic options
  $options_status = array(
    '1' => t('Enabled'),
    '0' => t('Disabled')
  );

  $form['language'] = array(
    '#type' => 'select',
    '#title' => t('Lenguage'),    '#description' => t('You can enable more languages from the domain settings.'),
    '#options' => $domain_languages,
    '#default_value' => 'en'
  );
  $form['script_status_indicator_type'] = array(
    '#type' => 'select',
    '#title' => t('Status Indicator Type'),
    '#options' => array(0 => t('Domain'), 1 => t('Agent')),
    '#default_value' => 0,
  );
  $form['script_agent'] = array(
    '#type' => 'select',
    '#title' => t('Agent'),
    '#options' => $agents_select,
  );
  $form['tracking'] = array(
    '#type' => 'radios',
    '#title' => t('Tracking'),
    '#options' => $options_status,
    '#default_value' => 1
  );
  $form['status_indicator'] = array(
    '#type' => 'radios',
    '#title' => t('Status indicator'),
    '#options' => $options_status,
    '#default_value' => 1
  );
  $form['tracking_script'] = array(
    '#attributes' => array(
      'readonly' => 'readonly',
      'onclick' => 'this.select();',
    ),
    '#type' => 'textarea',
    '#title' => t('Tracking script'),
    '#default_value' => $script,
    '#rows' => 10
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Back'),
  );

  return $form;
}

function activehelper_livehelp__domains_tracking_script_form_submit($form, &$form_state) {
  $form_state['redirect'] = 'admin/activehelper-livehelp/domains';

  return $form;
}

function activehelper_livehelp__domains_download_block($domain_id = 0) {
  global $activehelper_livehelp;

  $domain_id = (int) $domain_id;

  $domain = db_select('livehelp_domains', 'd')
      ->fields('d', array('id_domain', 'name', 'status'))
      ->condition('d.id_domain', $domain_id, '=')
      ->execute()
      ->fetchAssoc();

  module_load_include('inc', 'activehelper_livehelp', 'includes/livehelp_lib_files');

  // First: unzip the original external module
  activehelper_livehelp__lib_files_unzip($activehelper_livehelp['external_original_zip'], $activehelper_livehelp['base_dir']);

  // Second: replace the variables in there
  $block_file = $activehelper_livehelp['base_dir'] . '/activehelper_livehelp_external/activehelper_livehelp_external.module';

  $f = fopen($block_file, 'r');
  $output = fread($f, filesize($block_file));
  fclose($f);

  $output = str_replace('{__import_url__}', $activehelper_livehelp['import_url'], $output);
  $output = str_replace('{__domain_id__}', $domain_id, $output);
  $output = str_replace('{__domain_name__}', $domain['name'], $output);

  $f = fopen($block_file, 'w');
  fwrite($f, $output);
  fclose($f);

  // Third: zip the generated external module
  activehelper_livehelp__lib_files_zip($activehelper_livehelp['external_dir'], $activehelper_livehelp['external_zip']);

  // Fourth: delete the original module folder
  activehelper_livehelp__lib_files_delete($activehelper_livehelp['external_dir']);

  // Fifth: return the zip file to download
  header("Cache-Control: public");
  header("Content-Description: File Transfer");
  header("Content-Disposition: attachment; filename=" . basename($activehelper_livehelp['external_zip']));
  header("Content-Type: application/octet-stream");
  header("Content-Transfer-Encoding: binary");
  header("Content-Length: " . filesize($activehelper_livehelp['external_zip']));

  readfile($activehelper_livehelp['external_zip']);

  // Finally: we should not let Drupal print anything else.
  die;
}

function activehelper_livehelp__domains_settings($domain_id = 0, $domain_language = 'en') {
  $domain_id = (int) $domain_id;
  $domain_language = (string) $domain_language;

  $domain = db_select('livehelp_domains', 'd')
      ->fields('d', array('id_domain', 'name', 'status'))
      ->condition('d.id_domain', $domain_id, '=')
      ->execute()
      ->fetchAssoc();

  $output = '<h2>' . t('Domain settings for @domain', array('@domain' => $domain['name'])) . '</h2>';

  $form = drupal_get_form('activehelper_livehelp__domains_settings_form', $domain_id, $domain_language);
  $output .= render($form);

  $output .= '
		<script type="text/javascript">
			function livehelp_domain_language( select ) {
				window.location = "' . url('admin/activehelper-livehelp/domains/' . $domain_id) . '/" + select.options[ select.selectedIndex ].value + "/settings";
			}
		</script>
	';

  return $output;
}

function activehelper_livehelp__domains_settings_form($form, &$form_state) {
  global $activehelper_livehelp;

  $domain_id = (int) $form_state['build_info']['args'][0];
  $domain_language = (string) $form_state['build_info']['args'][1];

  $domain_pictures_url = sprintf($activehelper_livehelp['domains_pictures_url'], $domain_id, $domain_language);

  // Load the domain languages
  $domain_languages = array();
  $domain_languages_result = db_query("
		SELECT l.code, l.name, IF( ISNULL( ld.welcome_message ), 0, 1 ) AS status,
			IFNULL( ld.welcome_message, '' ) AS welcome_message
		FROM {livehelp_languages} AS l
			LEFT JOIN {livehelp_languages_domain} AS ld
				ON ( ld.id_domain = '{$domain_id}' AND ld.code = l.code )
	");
  while ($r = $domain_languages_result->fetchAssoc()) {
    $domain_languages[$r['code']] = $r;
  }

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

  // Load the domain settings in a results variable
  $domain_settings_result = db_query("
		SELECT name, value FROM {livehelp_settings}
		WHERE id_domain = '{$domain_id}'
	");
  // Insert the settings in a new array with the names as index
  $domain_settings = array();
  while ($r = $domain_settings_result->fetchAssoc()) {
    $domain_settings[$r['name']] = $r['value'];
  }

  // Basic options
  $options_status = array(
    '1' => t('Enabled'),
    '0' => t('Disabled')
  );

  $form['language'] = array(
    '#type' => 'fieldset',
    '#title' => t('Domain language'),
    '#weight' => 5,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE
  );
  $form['language']['domain_language'] = array(
    '#attributes' => array(
      'onchange' => 'livehelp_domain_language( this )'
    ),
    '#type' => 'select',
    '#title' => t('Lenguage of the images'),
    '#options' => $languages_texts,
    '#default_value' => $domain_language,
    '#description' => t('Select the language you would like to edit the images below for.'),
  );

  $form['general'] = array(
    '#type' => 'fieldset',
    '#title' => t('General'),
    '#weight' => 10,
    '#collapsible' => TRUE,
    '#collapsed' => TRUE
  );
  $form['general']['setting__' . 'livehelp_name'] = array(
    '#type' => 'textfield',
    '#title' => t('LiveHelp name'),
    '#default_value' => $domain_settings['livehelp_name'],
    '#size' => 60
  );
  $form['general']['setting__' . 'site_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Site name'),
    '#default_value' => $domain_settings['site_name'],
    '#size' => 60
  );
  $form['general']['setting__' . 'site_address'] = array(
    '#type' => 'textfield',
    '#title' => t('Site address'),
    '#default_value' => $domain_settings['site_address'],
    '#size' => 80
  );
  $form['general']['setting__' . 'departments'] = array(
    '#type' => 'radios',
    '#title' => t('Departments'),
    '#options' => $options_status,
    '#default_value' => $domain_settings['departments']
  );
  $form['general']['setting__' . 'disable_geolocation'] = array(
    '#type' => 'radios',
    '#title' => t('Disable geolocation'),
    '#options' => $options_status,
    '#default_value' => $domain_settings['disable_geolocation']
  );
  $form['general']['setting__' . 'disable_tracking_offline'] = array(
    '#type' => 'radios',
    '#title' => t('Disable status indicator in offline mode'),
    '#options' => $options_status,
    '#default_value' => $domain_settings['disable_tracking_offline']
  );
  $form['general']['setting__' . 'captcha'] = array(
    '#type' => 'radios',
    '#title' => t('Captcha'),
    '#options' => $options_status,
    '#default_value' => $domain_settings['captcha']
  );
  $form['general']['setting__' . 'company'] = array(
    '#type' => 'radios',
    '#title' => t('Company'),
    '#options' => $options_status,
    '#default_value' => $domain_settings['company']
  );
  $form['general']['setting__' . 'phone'] = array(
    '#type' => 'radios',
    '#title' => t('Phone'),
    '#options' => $options_status,
    '#default_value' => $domain_settings['phone']
  );

  $form['display'] = array(
    '#type' => 'fieldset',
    '#title' => t('Display'),
    '#weight' => 15,
    '#collapsible' => TRUE,
    '#collapsed' => TRUE
  );
  $form['display']['setting__' . 'background_color'] = array(
    '#type' => 'textfield',
    '#title' => t('Background color'),
    '#default_value' => $domain_settings['background_color'],
    '#size' => 60
  );
  $form['display']['setting__' . 'chat_font_type'] = array(
    '#type' => 'textfield',
    '#title' => t('Chat font type'),
    '#default_value' => $domain_settings['chat_font_type'],
    '#size' => 60
  );
  $form['display']['setting__' . 'guest_chat_font_size'] = array(
    '#type' => 'textfield',
    '#title' => t('Guest chat font size'),
    '#default_value' => $domain_settings['guest_chat_font_size'],
    '#size' => 60
  );
  $form['display']['setting__' . 'admin_chat_font_size'] = array(
    '#type' => 'textfield',
    '#title' => t('Admin chat font size'),
    '#default_value' => $domain_settings['admin_chat_font_size'],
    '#size' => 60
  );
  $form['display']['setting__' . 'disable_popup_help'] = array(
    '#type' => 'radios',
    '#title' => t('Disable popup help'),
    '#options' => $options_status,
    '#default_value' => $domain_settings['disable_popup_help']
  );

	$skins_dir = $activehelper_livehelp['base_dir'] . '/server/pictures/skins';
	$skins_dirs = array_filter(glob($skins_dir . '/*'), 'is_dir');

  $skins = array();
  foreach ($skins_dirs as $k => $v)
		$skins[basename($v)] = basename($v);

  $form['display']['setting__' . 'chat_background_img'] = array(
    '#type' => 'select',
    '#title' => t('Chat background'),
    '#options' => $skins,
    '#default_value' => $domain_settings['chat_background_img']
  );

  $form['display']['setting__' . 'campaign_link'] = array(
    '#type' => 'textfield',
    '#title' => t('Chat image link'),
    '#default_value' => $domain_settings['campaign_link'],
    '#size' => 80
  );
  $form['display']['setting__' . 'disable_agent_bannner'] = array(
    '#type' => 'radios',
    '#title' => t('Disable chat image'),
    '#options' => $options_status,
    '#default_value' => $domain_settings['disable_agent_bannner']
  );
  $img = $domain_pictures_url . '/' . $domain_settings['campaign_image'] . '?' . rand();
  $form['display']['campaign_image'] = array(
    '#type' => 'file',
    '#title' => t('Chat image'),
    '#size' => 80,
    '#description' => t('The already uploaded image:') . '<br /><br /><img src="' . $img . '" alt="" />'
  );
  $img = $domain_pictures_url . '/' . $domain_settings['chat_button_img'] . '?' . rand();
  $form['display']['chat_button_img'] = array(
    '#type' => 'file',
    '#title' => t('Chat send button'),
    '#size' => 80,
    '#description' => t('The already uploaded image:') . '<br /><br /><img src="' . $img . '" alt="" />'
  );
  $img = $domain_pictures_url . '/' . $domain_settings['chat_button_hover_img'] . '?' . rand();
  $form['display']['chat_button_hover_img'] = array(
    '#type' => 'file',
    '#title' => t('Chat send hand over'),
    '#size' => 80,
    '#description' => t('The already uploaded image:') . '<br /><br /><img src="' . $img . '" alt="" />'
  );

  $form['proactive'] = array(
    '#type' => 'fieldset',
    '#title' => t('Proactive'),
    '#weight' => 20,
    '#collapsible' => TRUE,
    '#collapsed' => TRUE
  );
  $img = $domain_pictures_url . '/' . $domain_settings['chat_invitation_img'] . '?' . rand();
  $form['proactive']['chat_invitation_img'] = array(
    '#type' => 'file',
    '#title' => t('Chat invitation image'),
    '#size' => 80,
    '#description' => t('The already uploaded image:') . '<br /><br /><img src="' . $img . '" alt="" />'
  );
  $form['proactive']['setting__' . 'invitation_refresh'] = array(
    '#type' => 'textfield',
    '#title' => t('Auto start invitation refresh (sec)'),
    '#default_value' => $domain_settings['invitation_refresh'],
    '#size' => 20
  );
  $form['proactive']['setting__' . 'disable_invitation'] = array(
    '#type' => 'radios',
    '#title' => t('Disable invitation'),
    '#options' => $options_status,
    '#default_value' => $domain_settings['disable_invitation']
  );

  $form['fonts'] = array(
    '#type' => 'fieldset',
    '#title' => t('Fonts'),
    '#weight' => 25,
    '#collapsible' => TRUE,
    '#collapsed' => TRUE
  );
  $form['fonts']['setting__' . 'font_type'] = array(
    '#type' => 'textfield',
    '#title' => t('Font type'),
    '#default_value' => $domain_settings['font_type'],
    '#size' => 60
  );
  $form['fonts']['setting__' . 'font_size'] = array(
    '#type' => 'textfield',
    '#title' => t('Font size'),
    '#default_value' => $domain_settings['font_size'],
    '#size' => 60
  );
  $form['fonts']['setting__' . 'font_color'] = array(
    '#type' => 'textfield',
    '#title' => t('Font color'),
    '#default_value' => $domain_settings['font_color'],
    '#size' => 60
  );
  $form['fonts']['setting__' . 'font_link_color'] = array(
    '#type' => 'textfield',
    '#title' => t('Font link color'),
    '#default_value' => $domain_settings['font_link_color'],
    '#size' => 60
  );
  $form['fonts']['setting__' . 'sent_font_color'] = array(
    '#type' => 'textfield',
    '#title' => t('Sent font color'),
    '#default_value' => $domain_settings['sent_font_color'],
    '#size' => 60
  );
  $form['fonts']['setting__' . 'received_font_color'] = array(
    '#type' => 'textfield',
    '#title' => t('Received font color'),
    '#default_value' => $domain_settings['received_font_color'],
    '#size' => 60
  );

  $form['chat'] = array(
    '#type' => 'fieldset',
    '#title' => t('Chat'),
    '#weight' => 30,
    '#collapsible' => TRUE,
    '#collapsed' => TRUE
  );
  $form['chat']['setting__' . 'disable_login_details'] = array(
    '#type' => 'radios',
    '#title' => t('Disable login'),
    '#options' => $options_status,
    '#default_value' => $domain_settings['disable_login_details']
  );
  $form['chat']['setting__' . 'disable_chat_username'] = array(
    '#type' => 'radios',
    '#title' => t('Disable chat username'),
    '#options' => $options_status,
    '#default_value' => $domain_settings['disable_chat_username']
  );
  $form['chat']['setting__' . 'require_guest_details'] = array(
    '#type' => 'radios',
    '#title' => t('Require guest details'),
    '#options' => $options_status,
    '#default_value' => $domain_settings['require_guest_details']
  );
  $form['chat']['setting__' . 'disable_language'] = array(
    '#type' => 'radios',
    '#title' => t('Disable language selection'),
    '#options' => $options_status,
    '#default_value' => $domain_settings['disable_language']
  );

  $form['email'] = array(
    '#type' => 'fieldset',
    '#title' => t('Email'),
    '#weight' => 35,
    '#collapsible' => TRUE,
    '#collapsed' => TRUE
  );
  $form['email']['setting__' . 'offline_email'] = array(
    '#type' => 'textfield',
    '#title' => t('Offline email'),
    '#default_value' => $domain_settings['offline_email'],
    '#size' => 60
  );
  $form['email']['setting__' . 'from_email'] = array(
    '#type' => 'textfield',
    '#title' => t('From email'),
    '#default_value' => $domain_settings['from_email'],
    '#size' => 60
  );
  $form['email']['setting__' . 'disable_offline_email'] = array(
    '#type' => 'radios',
    '#title' => t('Disable offline email'),
    '#options' => $options_status,
    '#default_value' => $domain_settings['disable_offline_email']
  );
  $form['email']['setting__' . 'custom_offline_form_link'] = array(
    '#type' => 'textfield',
    '#title' => t('Custom offline form'),
    '#default_value' => $domain_settings['custom_offline_form_link'],
    '#size' => 60
  );
  $form['email']['setting__' . 'log_offline_email'] = array(
    '#type' => 'radios',
    '#title' => t('Log offline message'),
    '#options' => $options_status,
    '#default_value' => $domain_settings['log_offline_email']
  );
  $form['email']['setting__' . 'configure_smtp'] = array(
    '#type' => 'radios',
    '#title' => t('SMTP'),
    '#options' => $options_status,
    '#default_value' => $domain_settings['configure_smtp']
  );
  $form['email']['setting__' . 'smtp_server'] = array(
    '#type' => 'textfield',
    '#title' => t('SMTP server'),
    '#default_value' => $domain_settings['smtp_server'],
    '#size' => 60
  );
  $form['email']['setting__' . 'smtp_port'] = array(
    '#type' => 'textfield',
    '#title' => t('SMTP port'),
    '#default_value' => $domain_settings['smtp_port'],
    '#size' => 60
  );

  $form['images'] = array(
    '#type' => 'fieldset',
    '#title' => t('Images'),
    '#weight' => 40,
    '#collapsible' => TRUE,
    '#collapsed' => TRUE
  );
  
 $form['images'] ['Images']  = array(
  array('#type' => 'link',
  '#title' => t('Get more chat buttons, themes and invitations here'),
  '#href' => 'http://www.activehelper.com/Icons/icon-store.html#free', 
  '#suffix' => '<br>'));
  
  $img = $domain_pictures_url . '/online.gif' . '?' . rand();
  $form['images']['image_online'] = array(
    '#type' => 'file',
    '#title' => t('Online image (gif)'),
    '#size' => 80,
    '#description' => t('The already uploaded image:') . '<br /><br /><img src="' . $img . '" alt="" />'
  );
  $img = $domain_pictures_url . '/offline.gif' . '?' . rand();
  $form['images']['image_offline'] = array(
    '#type' => 'file',
    '#title' => t('Offline image (gif)'),
    '#size' => 80,
    '#description' => t('The already uploaded image:') . '<br /><br /><img src="' . $img . '" alt="" />'
  );
  $img = $domain_pictures_url . '/away.gif' . '?' . rand();
  $form['images']['image_away'] = array(
    '#type' => 'file',
    '#title' => t('Away image (gif)'),
    '#size' => 80,
    '#description' => t('The already uploaded image:') . '<br /><br /><img src="' . $img . '" alt="" />'
  );
  $img = $domain_pictures_url . '/brb.gif' . '?' . rand();
  $form['images']['image_brb'] = array(
    '#type' => 'file',
    '#title' => t('Be right back image (gif)'),
    '#size' => 80,
    '#description' => t('The already uploaded image:') . '<br /><br /><img src="' . $img . '" alt="" />'
  );

  $form['languages'] = array(
    '#type' => 'fieldset',
    '#title' => t('Languages and welcome message'),
    '#weight' => 45,
    '#collapsible' => TRUE,
    '#collapsed' => TRUE
  );

  $first = TRUE;
  foreach ($languages_texts as $i_code => $v_text) {
    $form['languages'][$i_code] = array(
      '#type' => 'fieldset',
      '#title' => check_plain($v_text),
      '#collapsible' => TRUE,
      '#collapsed' => !isset($domain_languages[$i_code]) || $domain_languages[$i_code]['status'] != 1
    );
    $form['languages'][$i_code]['language__' . $i_code . '__status'] = array(
      '#name' => 'language_' . $i_code . '__status',
      '#type' => 'radios',
      '#title' => t('Status'),
      '#options' => $options_status,
      '#default_value' => isset($domain_languages[$i_code]) ? $domain_languages[$i_code]['status'] : '0'
    );
    $form['languages'][$i_code]['language__' . $i_code . '__welcome_message'] = array(
      '#type' => 'textfield',
      '#title' => t('Welcome message'),
      '#default_value' => isset($domain_languages[$i_code]) ? $domain_languages[$i_code]['welcome_message'] : '',
      '#size' => 100
    );
  }

  $form['rebranding'] = array(
    '#type' => 'fieldset',
    '#title' => t('Rebranding'),
    '#weight' => 50,
    '#collapsible' => TRUE,
    '#collapsed' => TRUE
  );
  $form['rebranding']['setting__' . 'disable_copyright'] = array(
    '#type' => 'radios',
    '#title' => t('Copyright'),
    '#options' => $options_status,
    '#default_value' => $domain_settings['disable_copyright']
  );
  $form['rebranding']['setting__' . 'copyright_image'] = array(
    '#type' => 'radios',
    '#title' => t('Image banner'),
    '#options' => $options_status,
    '#default_value' => $domain_settings['copyright_image']
  );
  $img = $domain_pictures_url . '/' . $domain_settings['company_logo'] . '?' . rand();
  $form['rebranding']['company_logo'] = array(
    '#type' => 'file',
    '#title' => t('Company image'),
    '#size' => 80,
    '#description' => t('The already uploaded image:') . '<br /><br /><img src="' . $img . '" alt="" />'
  );
  $form['rebranding']['setting__' . 'company_link'] = array(
    '#type' => 'textfield',
    '#title' => t('Company image link'),
    '#default_value' => $domain_settings['company_link'],
    '#size' => 60
  );
  $form['rebranding']['setting__' . 'company_slogan'] = array(
    '#type' => 'textfield',
    '#title' => t('Company slogan'),
    '#default_value' => $domain_settings['company_slogan'],
    '#size' => 60
  );

  $form['google_analytics'] = array(
    '#type' => 'fieldset',
    '#title' => t('Google Analytics integration'),
    '#weight' => 55,
    '#collapsible' => TRUE,
    '#collapsed' => TRUE
  );
  $form['google_analytics']['setting__' . 'analytics_account'] = array(
    '#type' => 'textfield',
    '#title' => t('Analytics account'),
    '#default_value' => $domain_settings['analytics_account'],
    '#size' => 60
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#weight' => 60,
    '#value' => t('Save'),
  );

  return $form;
}

function activehelper_livehelp__domains_settings_form_submit($form, &$form_state) {
  global $activehelper_livehelp;

  $form_state['redirect'] = 'admin/activehelper-livehelp/domains';

  $domain_id = (int) $form_state['build_info']['args'][0];
  $domain_language = (string) $form_state['build_info']['args'][1];

  $domain = db_select('livehelp_domains', 'd')
      ->fields('d', array('id_domain', 'name', 'status'))
      ->condition('d.id_domain', $domain_id, '=')
      ->execute()
      ->fetchAssoc();

  $domain_pictures_dir = sprintf($activehelper_livehelp['domains_pictures_dir'], $domain_id, $domain_language);

  // Load the current domain settings in a results variable
  $domain_settings_result = db_query("
		SELECT name, value FROM {livehelp_settings}
		WHERE id_domain = '{$domain_id}'
	");
  // Insert the settings in a new array with the names as index
  $domain_settings = array();
  while ($r = $domain_settings_result->fetchAssoc()) {
    $domain_settings[$r['name']] = $r['value'];
  }

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

  // Upload the campaign_image first
  if (isset($form_files['campaign_image']['error']) && $form_files['campaign_image']['error'] == 0) {
    $file = activehelper_livehelp__lib_images_upload(DRUPAL_ROOT . '/' . $domain_pictures_dir, 'chat_banner', $form_files['campaign_image'], '.gif');
    if (!is_array($file)) {
      // If $file is a string, it's an error.
      drupal_set_message(check_plain($file), 'error');
    }
  }

  $settings_images = array(
    'chat_button_img',
    'chat_button_hover_img',
    'chat_invitation_img',
    'company_logo'
  );
  foreach ($settings_images as $v) {
    if (isset($form_files[$v]['error']) && $form_files[$v]['error'] == 0) {
      $file = activehelper_livehelp__lib_images_upload(DRUPAL_ROOT . '/' . $domain_pictures_dir, $v, $form_files[$v]);
      if (!is_array($file)) {
        // If $file is a string, it's an error.
        drupal_set_message(check_plain($file), 'error');
      }
      else {
        if ($domain_settings[$v] != $file['basename']) {
          @unlink(DRUPAL_ROOT . '/' . $domain_pictures_dir . '/' . $domain_settings[$v]);
        }

        db_query("
					UPDATE {livehelp_settings}
					SET value = '" . $file['basename'] . "'
					WHERE name = '{$v}'
				");
      }
    }
  }

  $status_images = array(
    'image_online' => 'online',
    'image_offline' => 'offline',
    'image_away' => 'away',
    'image_brb' => 'brb'
  );
  foreach ($status_images as $i => $v) {
    if (isset($form_files[$i]['error']) && $form_files[$i]['error'] == 0) {
      $file = activehelper_livehelp__lib_images_upload(DRUPAL_ROOT . '/' . $domain_pictures_dir, $v, $form_files[$i], '.gif');
      if (!is_array($file)) {
        // If $file is a string, it's an error.
        drupal_set_message(check_plain($file), 'error');
      }
    }
  }

  // Languages values
  $domain_languages = array();
  foreach ($form_state['values'] as $i => $v) {
    // Update the settings table
    if (FALSE !== strpos($i, 'setting__')) {
      $i = str_replace('setting__', '', $i);

      db_query("
				UPDATE {livehelp_settings}
				SET value = '{$v}'
				WHERE name = '{$i}' AND id_domain = {$domain_id}
			");
    }
    // Update the languages table
    elseif (FALSE !== strpos($i, 'language__')) {
      // Possible indexes: language__en__status, language__en__welcome_message
      // List language code and the column name
      list ( $c, $i ) = explode('__', str_replace('language__', '', $i));
      if (!isset($domain_languages[$c])) {
        $domain_languages[$c] = array();
      }
      $domain_languages[$c][$i] = $v;
    }
  }

  // Update the languages table
  db_query("
		DELETE FROM {livehelp_languages_domain}
		WHERE Id_domain = {$domain_id}
	");
  foreach ($domain_languages as $i => $v) {
    if ($v['status']) {
      db_query("
				INSERT INTO {livehelp_languages_domain}
					( Id_domain, name, code, welcome_message )
				VALUES
					( {$domain_id}, '{$languages_texts[$i]}', '{$i}', '{$v['welcome_message']}' )
			");
    }
  }

  // Save the images
  drupal_set_message(t('The domain settings for @domain have been updated.', array('@domain' => $domain['name'])));

  return $form;
}

function activehelper_livehelp__domains_settings_add_defaults($domain_id) {
  $domain_id = (int) $domain_id;

  // Save the default settings for an specific domain ID.
  db_query("
		INSERT INTO {livehelp_settings}
			( name, value, id_domain )
		VALUES
			( 'admin_homepage', '/eserver1/panel/visitors_index.php', {$domain_id} ),
			( 'timezone', '+1000', {$domain_id} ),
			( 'default_department', 'General', {$domain_id} ),
			( 'departments', '1', {$domain_id} ),
			( 'disable_offline_email', '0', {$domain_id} ),
			( 'disable_login_details', '0', {$domain_id} ),
			( 'admin_chat_font_size', '12px', {$domain_id} ),
			( 'guest_chat_font_size', '12px', {$domain_id} ),
			( 'background_color', '#F9F9F9', {$domain_id} ),
			( 'font_link_color', '#333399', {$domain_id} ),
			( 'received_font_color', '#000000', {$domain_id} ),
			( 'sent_font_color', '#666666', {$domain_id} ),
			( 'chat_font_type', 'Arial, Arial Unicode, Lucida, Verdana', {$domain_id} ),
			( 'font_color', '#000000', {$domain_id} ),
			( 'font_size', '13px', {$domain_id} ),
			( 'font_type', 'Arial, Helvetica, sans-serif,Verdana', {$domain_id} ),
			( 'admin_smilies', '0', {$domain_id} ),
			( 'guest_smilies', '1', {$domain_id} ),
			( 'livehelp_logo', 'eserver/i18n/sp/pictures/help_logo.gif', {$domain_id} ),
			( 'livehelp_name', 'www.activehelper.com Live Help', {$domain_id} ),
			( 'offline_email', 'support@activehelper.com', {$domain_id} ),
			( 'site_address', 'http://www.activehelper.com', {$domain_id} ),
			( 'site_name', 'www.activehelper.com', {$domain_id} ),
			( 'initiate_chat_valign', 'top', {$domain_id} ),
			( 'initiate_chat_halign', 'right', {$domain_id} ),
			( 'disable_chat_username', '0', {$domain_id} ),
			( 'campaign_image', 'chat_banner.gif', {$domain_id} ),
			( 'campaign_link', 'http://www.activehelper.com/', {$domain_id} ),
			( 'disable_popup_help', '1', {$domain_id} ),
			( 'p3p', 'ALL DSP COR CUR OUR IND ONL UNI COM NAV', {$domain_id} ),
			( 'require_guest_details', '0', {$domain_id} ),
			( 'configure_smtp', '0', {$domain_id} ),
			( 'smtp_server', '', {$domain_id} ),
			( 'smtp_port', '25', {$domain_id} ),
			( 'from_email', 'support@activehelper.com', {$domain_id} ),
			( 'login_timeout', '20', {$domain_id} ),
			( 'chat_background_img', 'activehelper', {$domain_id} ),
			( 'chat_invitation_img', 'initiate_dialog.gif', {$domain_id} ),
			( 'chat_button_img', 'send.gif', {$domain_id} ),
			( 'chat_button_hover_img', 'send_hover.gif', {$domain_id} ),
			( 'custom_offline_form_link', '', {$domain_id} ),
			( 'log_offline_email', 0, {$domain_id} ),
			( 'disable_language', 0, {$domain_id} ),
			( 'company_logo', 'logo.jpg', {$domain_id} ),
			( 'company_link', 'http://www.activehelper.com', {$domain_id} ),
			( 'disable_copyright', 1, {$domain_id} ),
			( 'company_slogan', 'ACTIVEHELPER Platform All Rights Reserved', {$domain_id} ),
			( 'copyright_image', 1, {$domain_id} ),
			( 'analytics_account','', {$domain_id} ),
			( 'invitation_refresh', 0, {$domain_id} ),
			( 'disable_invitation', 0, {$domain_id} ),
			( 'disable_geolocation', 0, {$domain_id} ),
			( 'disable_tracking_offline', 0, {$domain_id} ),
			( 'captcha', 1, {$domain_id} ),
			( 'disable_agent_bannner', 0, {$domain_id} ),
      ( 'company', 0, {$domain_id} ),
      ( 'phone', 0, {$domain_id} )
	");
}

