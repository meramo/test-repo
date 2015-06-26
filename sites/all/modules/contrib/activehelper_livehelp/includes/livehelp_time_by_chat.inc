<?php

function activehelper_livehelp__time_by_chat() {
  global $activehelper_livehelp;

  $output = '<h2>Time by chat</h2>';

  drupal_add_css('div.form-item-from { float: left; margin-right: 25px; } .table-cell-nowrap { white-space: nowrap; }', 'inline');

  // Define the default values for the table sort
  if (!isset($_GET['order'])) {
    $_GET['order'] = t('Agent name');
  }
  if (!isset($_GET['sort'])) {
    $_GET['sort'] = 'asc';
  }
  if (!isset($_GET['page'])) {
    $_GET['page'] = 0;
  }

  $form = drupal_get_form('activehelper_livehelp__time_by_chat_date_form');
  $output .= render($form);

  if (isset($_GET['chat_delete'])) {
    activehelper_livehelp__time_by_chat_delete($_GET['chat_delete']);
  }

  switch ($_GET['order']) {
    case t('Agent name'):
      $order_field = "CONCAT( jlu.firstname, ' ', jlu.lastname )";
      break;
    case t('Domain name'):
      $order_field = "jld.name";
      break;
    case t('Visitor name'):
      $order_field = "jls.username";
      break;
    case t('Visitor email'):
      $order_field = "jls.email";
      break;
    case t('Rating'):
      $order_field = "IF( jls.rating = -1, '0', jls.rating )";
      break;
    case t('Duration'):
      $order_field = "TIMEDIFF( jls.refresh, jls.datetime )";
      break;
    case t('Date'):
      $order_field = "DATE_FORMAT( jls.datetime, '%m/%d/%Y' )";
      break;
    default:
      $order_field = "jls.id";
  }
  $order_type = $_GET['sort'];

  $limit_from = $_GET['page'] * $activehelper_livehelp['listing_limit'];
  $limit_to = ( $_GET['page'] + 1 ) * $activehelper_livehelp['listing_limit'];

  $date_from = $_GET['from'];
  $date_to = $_GET['to'];

  // Prepare table header
  $table_header = array(
    array('data' => t('ID'), 'field' => 'session_id'),
    array('data' => t('Agent name'), 'field' => 'agent_name', 'class' => array('table-cell-nowrap')),
    array('data' => t('Domain name'), 'field' => 'domain_name', 'class' => array('table-cell-nowrap')),
    array('data' => t('Visitor name'), 'field' => 'visitor_name', 'class' => array('table-cell-nowrap')),
    array('data' => t('Visitor email'), 'field' => 'visitor_email', 'class' => array('table-cell-nowrap')),
    array('data' => t('Rating'), 'field' => 'rating', 'class' => array('table-cell-nowrap')),
    array('data' => t('Duration'), 'field' => 'time_duration', 'class' => array('table-cell-nowrap')),
    array('data' => t('Date'), 'field' => 'date', 'class' => array('table-cell-nowrap')),
    array('data' => t('Operations'), 'class' => array('table-cell-nowrap'))
  );

  $query = "
		SELECT
			jls.id AS session_id, CONCAT( jlu.firstname, ' ', jlu.lastname ) AS agent_name, jld.name AS domain_name,
			jls.username AS visitor_name, jls.email AS visitor_email, IF( jls.rating = -1, '0', jls.rating ) AS rating,
			TIMEDIFF( jls.refresh, jls.datetime ) AS time_duration, DATE_FORMAT( jls.datetime, '%m/%d/%Y' ) AS date
		FROM
			{livehelp_sessions} AS jls,
			{livehelp_users} AS jlu,
			{livehelp_domains} AS jld
		WHERE
			DATE_FORMAT( jls.datetime, '%Y%m%d' ) >= '" . str_replace('-', '', $date_from) . "'
			AND DATE_FORMAT( jls.datetime,'%Y%m%d' ) <= '" . str_replace('-', '', $date_to) . "'
			AND jls.id_user = jlu.id
			AND jls.id_domain = jld.id_domain
		GROUP BY
			jls.id, jls.username
		ORDER BY
			" . $order_field . " " . $order_type . "
	";

  $limit_max = db_query("
		SELECT COUNT( result.session_id )
		FROM (" . $query . ") AS result
	")
      ->fetchField();

  // Set the query
  $result = db_query(
      $query . "
		LIMIT
			" . $limit_from . ", " . $limit_to . "
	");

  // Prepare table rows
  $table_rows = array();

  // Looping for filling the table rows
  while ($data = $result->fetchObject()) {
    // Fill the table rows
    $table_rows[] = array(
      $data->session_id,
      $data->agent_name,
      $data->domain_name,
      $data->visitor_name,
      $data->visitor_email,
      $data->rating,
      $data->time_duration,
      array('data' => $data->date, 'class' => array('table-cell-nowrap')),
      array(
        'data' =>
        l(
            t('View messages'), 'admin/activehelper-livehelp/chats/time-by-chat/' . $data->session_id . '/view-messages'
        ) . ' &nbsp;|&nbsp; ' .
        l(
            t('Delete'), 'admin/activehelper-livehelp/chats/time-by-chat', array(
          'attributes' => array(
            'onclick' => "return window.confirm( '" . t('Are you sure you want to delete this chat and all its messages?') . "' );"
          ),
          'query' => array(
            'chat_delete' => $data->session_id,
            'order' => $_GET['order'],
            'sort' => $_GET['sort'],
            'page' => $_GET['page'],
            'from' => $_GET['from'],
            'to' => $_GET['to']
          )
            )
        ),
        'class' => array('table-cell-nowrap')
      )
    );
  }

  pager_default_initialize($limit_max, $activehelper_livehelp['listing_limit']);

  // Render the table
  $output .= theme_table(array(
        'header' => $table_header,
        'rows' => $table_rows,
        'attributes' => array('width' => '100%'),
        'sticky' => TRUE,
        'caption' => '',
        'colgroups' => array(),
        'empty' => t('No chats found from @from to @to.', array('@from' => $date_from, '@to' => $date_to))
      ))
      . theme('pager');

  return $output;
}

function activehelper_livehelp__time_by_chat_date_form($form, &$form_state) {
  if (!isset($_GET['from']) || !preg_match('/^\d{4}-\d{2}-\d{2}$/', $_GET['from'])) {
    $_GET['from'] = (string) date('Y-m-d');
  }
  if (!isset($_GET['to']) || !preg_match('/^\d{4}-\d{2}-\d{2}$/', $_GET['to'])) {
    $_GET['to'] = (string) date('Y-m-d');
  }

  $date_from = explode('-', $_GET['from']);
  $date_to = explode('-', $_GET['to']);

  $form['controls'] = array(
    '#type' => 'fieldset',
    '#title' => t('Controls'),
    '#weight' => 10,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE
  );
  $form['controls']['from'] = array(
    '#type' => 'date',
    '#title' => t('From'),
    '#default_value' => array(
      'year' => intval($date_from[0], 10),
      'month' => intval($date_from[1], 10),
      'day' => intval($date_from[2], 10)
    )
  );
  $form['controls']['to'] = array(
    '#type' => 'date',
    '#title' => t('To'),
    '#default_value' => array(
      'year' => intval($date_to[0], 10),
      'month' => intval($date_to[1], 10),
      'day' => intval($date_to[2], 10)
    )
  );

  $form['controls']['submit_1'] = array(
    '#name' => 'op__apply',
    '#type' => 'submit',
    '#value' => t('Apply'),
  );
  $form['controls']['submit_2'] = array(
    '#name' => 'op__export',
    '#type' => 'submit',
    '#value' => t('Export all to CSV'),
  );

  return $form;
}

function activehelper_livehelp__time_by_chat_date_form_submit($form, &$form_state) {
  $date_from = date('Y-m-d');
  $date_to = $date_from;

  $from = &$form_state['values']['from'];
  if (checkdate($from['month'], $from['day'], $from['year'])) {
    $from['month'] = ( $from['month'] < 10 ) ? '0' . $from['month'] : $from['month'];
    $from['day'] = ( $from['day'] < 10 ) ? '0' . $from['day'] : $from['day'];
    $date_from = $from['year'] . '-' . $from['month'] . '-' . $from['day'];
  }

  $to = &$form_state['values']['to'];
  if (checkdate($to['month'], $to['day'], $to['year'])) {
    $to['month'] = ( $to['month'] < 10 ) ? '0' . $to['month'] : $to['month'];
    $to['day'] = ( $to['day'] < 10 ) ? '0' . $to['day'] : $to['day'];
    $date_to = $to['year'] . '-' . $to['month'] . '-' . $to['day'];
  }

  $_GET['from'] = $date_from;
  $_GET['to'] = $date_to;

  if (isset($form_state['values']['op__export'])) {
    activehelper_livehelp__time_by_chat_export_csv();
  }

  $query = array(
    'order' => $_GET['order'],
    'sort' => $_GET['sort'],
    'page' => $_GET['page'],
    'from' => $_GET['from'],
    'to' => $_GET['to']
  );
  drupal_goto('admin/activehelper-livehelp/chats/time-by-chat', array('query' => $query));

  return $form;
}

function activehelper_livehelp__time_by_chat_delete($session_id = 0) {
  $session_id = (int) $session_id;

  db_query("
		DELETE FROM {livehelp_sessions}, {livehelp_messages}
		USING {livehelp_sessions}
		INNER JOIN {livehelp_messages}
		WHERE {livehelp_sessions}.id = '{$session_id}'
			AND {livehelp_messages}.session = {livehelp_sessions}.id
	");

  drupal_set_message(t('The chat and its messages have been removed.'));
}

function activehelper_livehelp__time_by_chat_export_csv() {
  switch ($_GET['order']) {
    case t('Agent name'):
      $order_field = "CONCAT( jlu.firstname, ' ', jlu.lastname )";
      break;
    case t('Domain name'):
      $order_field = "jld.name";
      break;
    case t('Visitor name'):
      $order_field = "jls.username";
      break;
    case t('Visitor email'):
      $order_field = "jls.email";
      break;
    case t('Rating'):
      $order_field = "IF( jls.rating = -1, '0', jls.rating )";
      break;
    case t('Duration'):
      $order_field = "TIMEDIFF( jls.refresh, jls.datetime )";
      break;
    case t('Date'):
      $order_field = "DATE_FORMAT( jls.datetime, '%m/%d/%Y' )";
      break;
    default:
      $order_field = "jls.id";
  }
  $order_type = $_GET['sort'];

  $date_from = $_GET['from'];
  $date_to = $_GET['to'];

  // Prepare CSV header
  $output = '"' . t('Session ID') . '","' . t('Agent name') . '","'
      . t('Domain name') . '","' . t('Visitor name') . '","' . t('Visitor email') . '","'
      . t('Rating') . '","' . t('Duration') . '","' . t('Date') . '"';

  $query = "
		SELECT
			jls.id AS session_id, CONCAT( jlu.firstname, ' ', jlu.lastname ) AS agent_name, jld.name AS domain_name,
			jls.username AS visitor_name, jls.email AS visitor_email, IF( jls.rating = -1, '0', jls.rating ) AS rating,
			TIMEDIFF( jls.refresh, jls.datetime ) AS time_duration, DATE_FORMAT( jls.datetime, '%m/%d/%Y' ) AS date
		FROM
			{livehelp_sessions} AS jls,
			{livehelp_users} AS jlu,
			{livehelp_domains} AS jld
		WHERE
			DATE_FORMAT( jls.datetime, '%Y%m%d' ) >= '" . str_replace('-', '', $date_from) . "'
			AND DATE_FORMAT( jls.datetime,'%Y%m%d' ) <= '" . str_replace('-', '', $date_to) . "'
			AND jls.id_user = jlu.id
			AND jls.id_domain = jld.id_domain
		GROUP BY
			jls.id, jls.username
		ORDER BY
			" . $order_field . " " . $order_type . "
	";

  // Set the query
  $result = db_query($query);

  $rows = '';
  while ($data = $result->fetchObject()) {
    $rows .= "\n" . '"' . $data->session_id . '","' . $data->agent_name . '","' . $data->domain_name . '","'
        . $data->visitor_name . '","' . $data->visitor_email . '","' . $data->rating . '","' . $data->time_duration . '","' . $data->date . '"';
  }

  if ($rows == '') {
    $output .= "\n" . t('No chats found from @from to @to.', array('@from' => $date_from, '@to' => $date_to));
  }
  else {
    $output .= $rows;
  }

  header("Cache-Control: public");
  header("Content-Description: File Transfer");
  header("Content-Disposition: attachment; filename=LiveHelp_TimeByChat_" . $date_from . "~" . $date_to . ".csv");
  header("Content-Type: text/csv; text/comma-separated-values");
  header("Content-Transfer-Encoding: binary");

  echo $output;

  // Finally: we should not let Drupal print anything else.
  die;
}

function activehelper_livehelp__time_by_chat_view($session_id = 0) {
  $session_id = (int) $session_id;

  $output = '<h2>View messages</h2>';

  drupal_add_css('
		div.form-item-from { float: left; margin-right: 25px; }
		.table-cell-nowrap { white-space: nowrap; }
		.table-cell-full-width { width: 100%; }
		#activehelper-livehelp-time-by-chat-view-form .fieldset-wrapper { padding-top: 10px; }
	', 'inline');

  $form = drupal_get_form('activehelper_livehelp__time_by_chat_view_form', $session_id);
  $output .= render($form);

  // Prepare table header
  $table_header = array(
    array('data' => t('Full name'), 'class' => array('table-cell-nowrap')),
    array('data' => t('Message')),
    array('data' => t('Time'), 'class' => array('table-cell-nowrap')),
  );

  $query = "
		SELECT username AS fullname, message, TIME_FORMAT( jlm.datetime, '%H:%i:%s' ) AS time
		FROM {livehelp_messages} AS jlm
		WHERE session = '{$session_id}'
		ORDER BY id
	";

  // Set the query
  $result = db_query($query);

  // Prepare table rows
  $table_rows = array();

  // Looping for filling the table rows
  while ($data = $result->fetchObject()) {
    // Fill the table rows
    $table_rows[] = array(
      array('data' => $data->fullname, 'class' => array('table-cell-nowrap')),
      array('data' => $data->message, 'class' => array('table-cell-full-width')),
      array('data' => $data->time, 'class' => array('table-cell-nowrap')),
    );
  }

  $output .= '<table style="border: 0;"><tr><td style="border: 0; vertical-align: top; padding-top: 0; padding-left: 0;">';

  // Render the table
  $output .= theme_table(array(
    'header' => $table_header,
    'rows' => $table_rows,
    'attributes' => array('width' => '100%'),
    'sticky' => TRUE,
    'caption' => '',
    'colgroups' => array(),
    'empty' => t('No messages found in this chat.')
      ));


  $output .= '</td><td width="320" style="border: 0; vertical-align: top; padding-top: 0; padding-right: 0;">';


  // Prepare table header
  $table_header = array(
    t('Stat name'),
    t('Stat value'),
  );

  $query = "
		SELECT
			CONCAT( jlu.firstname, ' ', jlu.lastname ) AS agent_fullname, jls.department AS department_name, jls.server AS domain_name,
			jls.username AS visitor_name, jls.email AS visitor_email, DATE_FORMAT( jls.datetime, '%Y-%m-%d' ) AS date,
			IF( jls.rating = -1, '0', jls.rating ) AS rating, jlr.country, jlr.city
		FROM
			{livehelp_sessions} AS jls,
			{livehelp_users} AS jlu,
			{livehelp_requests} AS jlr
		WHERE jls.id = '{$session_id}'
			AND jls.id_user = jlu.id
			AND jls.request = jlr.id
		LIMIT 1
	";

  // Set the query
  $result = db_query($query);

  // Prepare table rows
  $table_rows = array();

  // Looping for filling the table rows
  while ($data = $result->fetchObject()) {
    // Fill the table rows
    $table_rows[] = array(
      t('Domain name'),
      $data->domain_name,
    );
    $table_rows[] = array(
      t('Department name'),
      $data->department_name,
    );
    $table_rows[] = array(
      t('Agent name'),
      $data->agent_fullname,
    );
    $table_rows[] = array(
      t('Visitor name'),
      $data->visitor_name,
    );
    $table_rows[] = array(
      t('Visitor email'),
      $data->visitor_email,
    );
    $table_rows[] = array(
      t('Country'),
      $data->country,
    );
    $table_rows[] = array(
      t('City'),
      $data->city,
    );
    $table_rows[] = array(
      t('Date'),
      $data->date,
    );
    $table_rows[] = array(
      t('Rating'),
      $data->rating,
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
    'empty' => t('No stats found in this chat.')
      ));

  $output .= '</td></tr></table>';

  return $output;
}

function activehelper_livehelp__time_by_chat_view_form($form, &$form_state) {
  $form['controls'] = array(
    '#type' => 'fieldset',
    '#title' => t('Controls'),
    '#weight' => 10,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE
  );

  $form['controls']['submit_1'] = array(
    '#name' => 'op__email',
    '#type' => 'submit',
    '#value' => t('Send as email'),
  );
  $form['controls']['submit_2'] = array(
    '#name' => 'op__export',
    '#type' => 'submit',
    '#value' => t('Export all to CSV'),
  );

  return $form;
}

function activehelper_livehelp__time_by_chat_view_form_submit($form, &$form_state) {
  $session_id = (int) $form_state['build_info']['args'][0];

  if (isset($form_state['values']['op__export'])) {
    activehelper_livehelp__time_by_chat_view_export_csv($session_id);
  }

  drupal_goto('admin/activehelper-livehelp/chats/time-by-chat/' . $session_id . '/email-messages');

  return $form;
}

function activehelper_livehelp__time_by_chat_view_export_csv($session_id = 0) {
  $session_id = (int) $session_id;

  $output = '';

  // Prepare CSV header
  $output .= '"' . t('Domain name') . '","' . t('Department name') . '","' . t('Agent name') . '","'
      . t('Visitor name') . '","' . t('Visitor email') . '","' . t('Country') . '","'
      . t('City') . '","' . t('Date') . '","' . t('Rating') . '"';

  $query = "
		SELECT
			CONCAT( jlu.firstname, ' ', jlu.lastname ) AS agent_fullname, jls.department AS department_name, jls.server AS domain_name,
			jls.username AS visitor_name, jls.email AS visitor_email, DATE_FORMAT( jls.datetime, '%Y-%m-%d' ) AS date,
			IF( jls.rating = -1, '0', jls.rating ) AS rating, jlr.country, jlr.city
		FROM
			{livehelp_sessions} AS jls,
			{livehelp_users} AS jlu,
			{livehelp_requests} AS jlr
		WHERE jls.id = '{$session_id}'
			AND jls.id_user = jlu.id
			AND jls.request = jlr.id
		LIMIT 1
	";

  // Set the query
  $result = db_query($query);

  $rows = '';
  $filename = 'View';
  while ($data = $result->fetchObject()) {
    $filename = $data->domain_name . '_' . $data->date . '_' . str_replace(' ', '-', $data->agent_fullname) . '_' . str_replace(' ', '-', $data->visitor_name);
    $rows .= "\n" . '"' . $data->domain_name . '","' . $data->department_name . '","' . $data->agent_fullname . '","'
        . $data->visitor_name . '","' . $data->visitor_email . '","' . $data->country . '","'
        . $data->city . '","' . $data->date . '","' . $data->rating . '"';
  }

  if ($rows == '') {
    $output .= "\n" . t('No stats found in this chat.');
  }
  else {
    $output .= $rows;
  }


  $output .= "\n\n";


  // Prepare CSV header
  $output .= '"' . t('Full name') . '","' . t('Message') . '","' . t('Time') . '"';

  $query = "
		SELECT username AS fullname, message, TIME_FORMAT( jlm.datetime, '%H:%i:%s' ) AS time
		FROM {livehelp_messages} AS jlm
		WHERE session = '{$session_id}'
		ORDER BY id
	";

  // Set the query
  $result = db_query($query);

  $rows = '';
  while ($data = $result->fetchObject()) {
    $rows .= "\n" . '"' . $data->fullname . '","' . str_replace('"', '\"', $data->message) . '","' . $data->time . '"';
  }

  if ($rows == '') {
    $output .= "\n" . t('No messages found in this chat.');
  }
  else {
    $output .= $rows;
  }

  header("Cache-Control: public");
  header("Content-Description: File Transfer");
  header("Content-Disposition: attachment; filename=LiveHelp_TimeByChat_" . $filename . ".csv");
  header("Content-Type: text/csv; text/comma-separated-values");
  header("Content-Transfer-Encoding: binary");

  echo $output;

  // Finally: we should not let Drupal print anything else.
  die;
}

function activehelper_livehelp__time_by_chat_email($session_id = 0) {
  $session_id = (int) $session_id;

  $output = '<h2>Email messages</h2>';

  $form = drupal_get_form('activehelper_livehelp__time_by_chat_email_form', $session_id);
  $output .= render($form);

  return $output;
}

function activehelper_livehelp__time_by_chat_email_form($form, &$form_state) {
  $session_id = (int) $form_state['build_info']['args'][0];

  $query = "
		SELECT
			CONCAT( jlu.firstname, ' ', jlu.lastname ) AS agent_fullname, jls.department AS department_name, jls.server AS domain_name,
			jls.username AS visitor_name, jls.email AS visitor_email, DATE_FORMAT( jls.datetime, '%Y-%m-%d' ) AS date,
			IF( jls.rating = -1, '0', jls.rating ) AS rating, jlr.country, jlr.city
		FROM
			{livehelp_sessions} AS jls,
			{livehelp_users} AS jlu,
			{livehelp_requests} AS jlr
		WHERE jls.id = '{$session_id}'
			AND jls.id_user = jlu.id
			AND jls.request = jlr.id
		LIMIT 1
	";
  $result = db_query($query);
  $stats = $result->fetchObject();

  $form['subject'] = array(
    '#type' => 'textfield',
    '#title' => t('Subject'),
    '#size' => 80,
    '#required' => TRUE,
    '#default_value' => t('Live Help - Time by chat: Within @agent_fullname and @visitor_name on @date', array('@agent_fullname' => $stats->agent_fullname, '@visitor_name' => $stats->visitor_name, '@date' => $stats->date))
  );
  $form['emisor_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Emisor name'),
    '#size' => 40,
    '#required' => TRUE,
    '#description' => t('Default is visitor name'),
    '#default_value' => $stats->visitor_name
  );
  $form['emisor_email'] = array(
    '#type' => 'textfield',
    '#title' => t('Emisor email'),
    '#size' => 30,
    '#required' => TRUE,
    '#description' => t('Default is visitor email'),
    '#default_value' => $stats->visitor_email
  );
  $form['recipient_email'] = array(
    '#type' => 'textfield',
    '#title' => t('Recipient email'),
    '#size' => 30,
    '#required' => TRUE
  );


  $message = '' . t('Stats') . "\n";

  $message .= "\n" . '' . t('Domain name') . ': ' . $stats->domain_name;
  $message .= "\n" . '' . t('Department name') . ': ' . $stats->department_name;
  $message .= "\n" . '' . t('Agent name') . ': ' . $stats->agent_fullname;
  $message .= "\n" . '' . t('Visitor name') . ': ' . $stats->visitor_name;
  $message .= "\n" . '' . t('Visitor email') . ': ' . $stats->visitor_email;
  $message .= "\n" . '' . t('Country') . ': ' . $stats->country;
  $message .= "\n" . '' . t('City') . ': ' . $stats->city;
  $message .= "\n" . '' . t('Date') . ': ' . $stats->date;
  $message .= "\n" . '' . t('Rating') . ': ' . $stats->rating;

  $query = "
		SELECT username AS fullname, message, TIME_FORMAT( jlm.datetime, '%H:%i:%s' ) AS time
		FROM {livehelp_messages} AS jlm
		WHERE session = '{$session_id}'
		ORDER BY id
	";
  $result = db_query($query);

  $message .= "\n\n" . '' . t('Messages') . "\n";

  $rows = '';
  while ($data = $result->fetchObject()) {
    $rows .= "\n" . '[' . $data->time . '] ' . $data->fullname . ': ' . htmlspecialchars($data->message) . '';
  }

  if ($rows == '') {
    $message .= "\n" . t('No messages found in this chat.');
  }
  else {
    $message .= $rows;
  }

  $form['message'] = array(
    '#type' => 'textarea',
    '#title' => t('Message'),
    '#default_value' => $message,
    '#rows' => 20
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Send'),
  );

  return $form;
}

function activehelper_livehelp__time_by_chat_email_form_submit($form, &$form_state) {
  $session_id = (int) $form_state['build_info']['args'][0];

  $form_state['redirect'] = 'admin/activehelper-livehelp/chats/time-by-chat/' . $session_id . '/view-messages';

  $module = 'activehelper_livehelp';
  $key = 'time_by_chat_email';

  $mail_to = $form_state['values']['recipient_email'];
  $mail_language = language_default();
  $mail_params = array();
  $mail_from = $form_state['values']['emisor_name'] . ' <' . $form_state['values']['emisor_email'] . '>';

  $result = drupal_mail($module, $key, $mail_to, $mail_language, $mail_params, $mail_from, FALSE);
  $result['subject'] = $form_state['values']['subject'];
  $result['body'] = array($form_state['values']['message']);

  $system = drupal_mail_system($module, $key);
  $result = $system->format($result);

  if ($system->mail($result)) {
    drupal_set_message(t('The form has been sent as an email to @email.', array('@email' => $mail_to)));
  }
  else {
    drupal_set_message(t('The form has not been sent as an email due to an internal error.'), 'error');
  }

  return $form;
}

