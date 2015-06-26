<?php

function activehelper_livehelp__chats_by_keyword() {
  global $activehelper_livehelp;

  $output = '<h2>Chats by keyword</h2>';

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
  if (!isset($_GET['keyword'])) {
    $_GET['keyword'] = '';
  }
  $_GET['keyword'] = htmlspecialchars((string) $_GET['keyword']);

  $form = drupal_get_form('activehelper_livehelp__chats_by_keyword_form');
  $output .= render($form);

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
			{livehelp_messages} AS jlm,
			{livehelp_sessions} AS jls,
			{livehelp_users} AS jlu,
			{livehelp_domains} AS jld
		WHERE
			LOWER( jlm.message ) LIKE LOWER( '%" . $_GET['keyword'] . "%' )
			AND jls.id = jlm.session
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
            t('View messages'), 'admin/activehelper-livehelp/chats/time-by-chat/' . $data->session_id . '/view-messages', array('attributes' => array('target' => '_blank'))
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
        'empty' => t('No chats found with messages that match the given keyword.')
      ))
      . theme('pager');

  return $output;
}

function activehelper_livehelp__chats_by_keyword_form($form, &$form_state) {
  $form['controls'] = array(
    '#type' => 'fieldset',
    '#title' => t('Controls'),
    '#weight' => 10,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE
  );
  $form['controls']['keyword'] = array(
    '#type' => 'textfield',
    '#title' => t('Keyword'),
    '#size' => 45,
    '#required' => TRUE,
    '#default_value' => $_GET['keyword']
  );

  $form['controls']['submit_1'] = array(
    '#name' => 'op__apply',
    '#type' => 'submit',
    '#value' => t('Apply'),
  );
  if ($_GET['keyword'] != '') {
    $form['controls']['submit_2'] = array(
      '#name' => 'op__export',
      '#type' => 'submit',
      '#value' => t('Export all to CSV'),
    );
  }

  return $form;
}

function activehelper_livehelp__chats_by_keyword_form_submit($form, &$form_state) {
  $_GET['keyword'] = &$form_state['values']['keyword'];

  if (isset($form_state['values']['op__export'])) {
    activehelper_livehelp__chats_by_keyword_export_csv();
  }

  $query = array(
    'order' => $_GET['order'],
    'sort' => $_GET['sort'],
    'page' => $_GET['page'],
    'keyword' => $_GET['keyword']
  );
  drupal_goto('admin/activehelper-livehelp/chats/chats-by-keyword', array('query' => $query));

  return $form;
}

function activehelper_livehelp__chats_by_keyword_export_csv() {
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
			{livehelp_messages} AS jlm,
			{livehelp_sessions} AS jls,
			{livehelp_users} AS jlu,
			{livehelp_domains} AS jld
		WHERE
			LOWER( jlm.message ) LIKE LOWER( '%" . $_GET['keyword'] . "%' )
			AND jls.id = jlm.session
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
    $output .= "\n" . t('No chats found with messages that match the given keyword.');
  }
  else {
    $output .= $rows;
  }

  header("Cache-Control: public");
  header("Content-Description: File Transfer");
  header("Content-Disposition: attachment; filename=LiveHelp_ChatsByKeyword_" . str_replace(' ', '-', $_GET['keyword']) . ".csv");
  header("Content-Type: text/csv; text/comma-separated-values");
  header("Content-Transfer-Encoding: binary");

  echo $output;

  // Finally: we should not let Drupal print anything else.
  die;
}

