<?php

function activehelper_livehelp__restricted_countries() {
  global $activehelper_livehelp;

  $output = '<h2>' . t('Not allowed countries') . '</h2>';

  // Define the default values for the table sort
  if (!isset($_GET['order'])) {
    $_GET['order'] = t('Restriction ID');
  }
  if (!isset($_GET['sort'])) {
    $_GET['sort'] = 'asc';
  }
  if (!isset($_GET['page'])) {
    $_GET['page'] = 0;
  }

  if (isset($_GET['restriction_delete'])) {
    activehelper_livehelp__restricted_countries_delete($_GET['restriction_delete']);
  }

  switch ($_GET['order']) {
    case t('Country'):
      $order_field = "jlc.name";
      break;
    case t('Domain name'):
      $order_field = "jld.name";
      break;
    case t('Domain ID'):
      $order_field = "jld.id_domain";
      break;
    case t('Restriction ID'):
    default:
      $order_field = "jlnac.id";
  }
  $order_type = $_GET['sort'];

  $limit_from = $_GET['page'] * $activehelper_livehelp['listing_limit'];
  $limit_to = ( $_GET['page'] + 1 ) * $activehelper_livehelp['listing_limit'];

  // Prepare table header
  $table_header = array(
    array('data' => t('Restriction ID'), 'field' => 'restriction_id'),
    array('data' => t('Domain ID'), 'field' => 'domain_id'),
    array('data' => t('Domain name'), 'field' => 'domain_name'),
    array('data' => t('Country'), 'field' => 'country'),
    t('Operations')
  );

  $query = "
		SELECT jlnac.id AS restriction_id, jld.id_domain AS domain_id, jld.name AS domain_name, jlc.name AS country
		FROM  {livehelp_not_allowed_countries} AS jlnac, {livehelp_countries} AS jlc, {livehelp_domains} AS jld
		WHERE jlnac.id_domain = jld.id_domain AND jlnac.code = jlc.code
		GROUP BY 1 DESC
		ORDER BY
			" . $order_field . " " . $order_type . "
	";

  $limit_max = db_query("
		SELECT COUNT( result.restriction_id )
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
      $data->restriction_id,
      $data->domain_id,
      $data->domain_name,
      $data->country,
      l(
          t('Delete'), 'admin/activehelper-livehelp/restricted-countries', array(
        'attributes' => array(
          'onclick' => "return window.confirm( '" . t('Are you sure you want to delete the restriction with ID @restriction_id?', array('@restriction_id' => $data->restriction_id)) . "' );"
        ),
        'query' => array(
          'restriction_delete' => $data->restriction_id
        )
          )
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
        'empty' => t('No restrictions found.')
      ))
      . theme('pager');

  return $output;
}

function activehelper_livehelp__restricted_countries_add() {
  global $activehelper_livehelp;

  $output = '<h2>' . t('Add restriction') . '</h2>';

  $form = drupal_get_form('activehelper_livehelp__restricted_countries_add_form');
  $output .= render($form);

  return $output;
}

function activehelper_livehelp__restricted_countries_add_form($form, &$form_state) {
  $query = db_select('livehelp_domains', 'd')
      ->fields('d', array('id_domain', 'name', 'status'));
  $result = $query->execute();

  $domains = array();
  while ($data = $result->fetchObject()) {
    $domains[$data->id_domain] = $data->name;
  }

  $query = db_select('livehelp_countries', 'c')
      ->fields('c', array('code', 'name'))
      ->orderBy('name');
  $result = $query->execute();

  $countries = array();
  while ($data = $result->fetchObject()) {
    $countries[$data->code] = $data->name;
  }

  $form['domain'] = array(
    '#type' => 'select',
    '#title' => t('Domain'),
    '#required' => TRUE,
    '#options' => $domains
  );
  $form['country'] = array(
    '#type' => 'select',
    '#title' => t('Country'),
    '#required' => TRUE,
    '#options' => $countries
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

function activehelper_livehelp__restricted_countries_add_form_submit($form, &$form_state) {
  db_query("
		INSERT INTO {livehelp_not_allowed_countries}
			( id_domain, code )
		VALUES
			( '{$form_state['values']['domain']}', '{$form_state['values']['country']}' )
	");

  $form_state['redirect'] = 'admin/activehelper-livehelp/restricted-countries';

  drupal_set_message(t('The restriction has been registered.'));
}

function activehelper_livehelp__restricted_countries_delete($restriction_id = 0) {
  $restriction_id = (int) $restriction_id;

  db_query("
		DELETE FROM  {livehelp_not_allowed_countries}
		WHERE id = {$restriction_id}
		LIMIT 1
	");

  drupal_set_message(t('The restriction has been removed.'));
}

